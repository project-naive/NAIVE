/*****************************************************************
*	This section defines a class that manages 2D text as textures.
*	It is also responsible for drawing the text when needed.
*	
*
*/


#pragma once

#include "..\..\Text\FTF.h"

#include <GL\glew.h>
#include <glm\glm.hpp>

#include "..\Globals\ContextManager.h"
#include "ShaderManager.h"
#include "..\..\Models\Text\Text.h"

namespace Engine {
	namespace Graphics {
		namespace Managers {
			class Text {
			public:
				Text(Context& CManager, Shader& SManager);
				~Text();
			private:
				Context& ContextManager;
				Shader& ShaderManager;
				Models::Text TextModel;
				//The Instance Manager of FreeTypeFont APIs
				FTFonts font = FTFonts();
				//The face index interface with the manager
				size_t* font_index = nullptr;
			public:
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
			private:
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
				glm::mat4 projection{};
			public:
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

				//Functions to render given text
				void UpdateDisplay(glm::mat4 in_projection) {
					projection = in_projection;
				}
				//Call this function to render random UTF-32 encoded strings
				//Default length 0 means the string is NULL terminated
				void renderText(const uint32_t* text, glm::vec2 pos, glm::vec4 color, size_t length = 0, size_t font_index = 0, GLfloat scale = 1);
				//Currently only support ASCII rendering, may add encoding support in the future
				//Default length 0 means the string is NULL terminated
				void renderText(const char* text, glm::vec2 pos, glm::vec4 color, size_t length = 0, size_t font_index = 0, GLfloat scale = 1);
			};
		}
	}
}

