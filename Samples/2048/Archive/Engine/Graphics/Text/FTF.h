#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Engine {
	namespace Graphics {
		class FTFonts {
		public:
			FTFonts();
			~FTFonts();
			bool initialized = false;
			FT_Library library;
		public:
			FT_Int version_major;
			FT_Int version_minor;
			FT_Int version_patch;

			size_t load_face(const FT_Byte* data, size_t size, size_t index = 0);
			bool unload_face(const size_t index);
			FT_Face& get_face(size_t index);

			//face_count includes unloaded indexes
			FT_Face* faces = nullptr;
			size_t face_count = 0;
			size_t face_cache = 0;
			size_t* unloaded = 0;
			size_t unload_count = 0;
			size_t unload_cache = 0;
		private:
			bool resize_faces();
		public:
			bool reserve_faces(size_t size);
		};
	}
}