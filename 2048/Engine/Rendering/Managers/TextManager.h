/*****************************************************************
*	This section defines a class that manages 2D text as textures.
*	It is also responsible for drawing the text when needed.
*	
*
*/


#pragma once

#include "..\Text\FTF.h"

#include <GL\glew.h>
#include <glm\glm.hpp>

#include "Manager.h"

namespace Engine {
	namespace Rendering {
		namespace Managers {
			class Text:public Manager {
			public:
				void Init() override {
					font = new FTFonts;
				}
				void Destroy() override;

				//The Instance Manager of FreeTypeFont APIs
				FTFonts* font = nullptr;
				//The face index interface with the manager
				size_t* font_index = nullptr;

				//The struct to store info about a given glyph as texture
				struct GlyphTexture {
					GLuint TextureID;
					glm::ivec2 size;
					glm::ivec2 bearing;
					GLuint advance;
				};

				//The struct to manage one face of font
				struct GlyphFont {
					unsigned start = 0;
					unsigned length = 0;
					unsigned pixel_height = 0;
					GlyphTexture* glyphs = nullptr;
				};

				//All of the faces of fonts loaded
				GlyphFont* FontTextures;
				//The values to maintain a dynamic array of textures
				size_t texture_cache = 0;
				size_t texture_count = 0;
				size_t* unloaded = nullptr;
				size_t unload_cache = 0;
				size_t unload_count = 0;
				//resizes the array of textures
				bool resize_texture();
				//reserves memory for textures
				bool reserve_texture(size_t size);

				//Interface to load font from memory
				size_t loadFace(const char* data, size_t size, size_t index = 0);
				//Frees the FTF resources for given texture
				bool unloadFace(size_t texture_index = 0);
				//Deletes the set to textures of given font
				bool unloadTexture(size_t texture_index = 0);
				//Generates a font set for given range of char code
				void genCharTexture(unsigned char_start, unsigned number, unsigned pixel_height, size_t texture_index = 0);

				//Function to render given text
	//			void renderText(const char* text, size_t texture_index, glm::vec2 pos, glm::vec4 color);
			};
		}
	}
}

