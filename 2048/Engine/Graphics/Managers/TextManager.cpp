#include "TextManager.h"

#include <iostream>
#include <algorithm>


namespace Engine {
	namespace Graphics {
		namespace Managers {
			Text::~Text() {
				std::sort(unloaded, unloaded + unload_count);
				size_t j = 0;
				for (size_t i = 0; i < texture_count; i++) {
					if (i == unloaded[j]) j++;
					else {
						for (unsigned k = 0; k < FontTextures[i].length; k++) {
							glDeleteTextures(1, &FontTextures[i].glyphs[k].TextureID);
						}
						delete[] FontTextures[i].glyphs;
					}
				}
				delete[] FontTextures;
				delete[] unloaded;
				delete[] font_index;
			}

			bool Text::reserve_texture(size_t size) {
				if (!size) return false;
				if (size < texture_count) return false;
				GlyphFont* new_Textures{ nullptr };
				size_t* new_index{ nullptr };
				try {
					new_Textures = new GlyphFont[size];
					new_index = new size_t[size];
				} catch (...) {
					delete[] new_Textures, new_index;
					return false;
				}
				for (size_t i = 0; i < texture_count; i++) {
					new_Textures[i] = FontTextures[i];
					new_index[i] = font_index[i];
				}
				std::swap(FontTextures, new_Textures);
				std::swap(font_index, new_index);
				texture_cache = size;
				delete[] new_Textures, new_index;
				return true;
			}

			bool Text::resize_texture() {
				GlyphFont* new_textures{ nullptr };
				size_t* new_index{ nullptr };
				try {
					size_t new_cache = size_t(texture_cache * 1.618);
					new_cache++;
					new_textures = new GlyphFont[new_cache];
					new_index = new size_t[new_cache];
					for (size_t i = 0; i < texture_count; i++) {
						new_textures[i] = FontTextures[i];
						new_index[i] = font_index[i];
					}
					std::swap(FontTextures, new_textures);
					std::swap(font_index, new_index);
					texture_cache = new_cache;
					delete[] new_textures, new_index;
					return true;
				} catch (...) {
					delete[] new_textures, new_index;
					return false;
				}
			}

			size_t Text::loadFace(const char* data, size_t size, size_t index) {
				size_t new_font_index = font.load_face((const FT_Byte*)data, size, index);
				if (new_font_index == size_t(-1)) {
					return -1;
				}
				if (unload_count) {
					unload_count--;
					size_t rtn = unloaded[unload_count];
					font_index[rtn] = new_font_index;
					return rtn;
				}
				if (texture_count == texture_cache) {
					if (!resize_texture()) {
						return -1;
					}
				}
				size_t rtn = texture_count;
				font_index[rtn] = new_font_index;
				return rtn;
			}

			bool Graphics::Managers::Text::unloadFace(size_t texture_index) {
				if (!font.unload_face(font_index[texture_index])) {
					return false;
				}
				font_index[texture_index] = -1;
				return true;
			}

			bool Text::unloadTexture(size_t texture_index) {
				if (unload_count == unload_cache) {
					size_t new_cache = size_t(unload_cache*1.618);
					new_cache++;
					size_t* new_buffer = nullptr;
					try {
						new_buffer = new size_t[new_cache];
						for (size_t i = 0; i < unload_count; i++) {
							new_buffer[i] = unloaded[i];
						}
					} catch (...) {
						delete[] new_buffer;
						return false;
					}
					std::swap(unloaded, new_buffer);
					unload_cache = new_cache;
					delete[] new_buffer;
				}
				if (font_index[texture_index] != size_t(-1)) {
					if (!font.unload_face(font_index[texture_index])) {
						return false;
					}
					font_index[texture_index] = -1;
				}
				for (size_t i = 0; i < FontTextures[texture_index].length; i++) {
					glDeleteTextures(1, &( FontTextures[texture_index].glyphs[i].TextureID ));
				}
				delete[] FontTextures[texture_index].glyphs;
				unloaded[unload_count] = texture_index;
				unload_count++;
				return true;
			}

			void Text::genCharTexture(
				unsigned char_start,
				unsigned number,
				unsigned pixel_height,
				size_t texture_index) {
				FT_Face face = font.faces[font_index[texture_index]];
				FT_Error err = FT_Set_Pixel_Sizes(face, 0, pixel_height);
				if (err) {
					std::cerr << "Failed to set pixel sizes!\nError code: " << err << '\n' << std::endl;
					return;
				}
				FontTextures[texture_index].start = char_start;
				FontTextures[texture_index].length = number;
				FontTextures[texture_index].pixel_height = pixel_height;
				FontTextures[texture_index].glyphs = new GlyphTexture[number];
				for (unsigned i = 0; i < number; i++) {
					err = FT_Load_Char(face, char_start + i, FT_LOAD_RENDER);
					if (err) {
						std::cerr << "Failed to load char with code " << char_start + i << "\nError code: " << err << '\n' << std::endl;
					}
					GLuint texture;
					glGenTextures(1, &texture);
					glBindTexture(GL_TEXTURE_2D, texture);
					glTexImage2D(
						GL_TEXTURE_2D,
						0,
						GL_RED,
						face->glyph->bitmap.width,
						face->glyph->bitmap.rows,
						0,
						GL_RED,
						GL_UNSIGNED_BYTE,
						face->glyph->bitmap.buffer
					);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					FontTextures[texture_index].glyphs[i] = {
						texture,
						glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
						glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
						GLuint(face->glyph->advance.x)
					};
				}
			}
		}
	}
}