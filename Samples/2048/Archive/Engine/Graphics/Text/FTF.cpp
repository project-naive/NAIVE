#include "FTF.h"

#include <iostream>
#include <algorithm>

namespace Engine {
	namespace Graphics {
		FTFonts::FTFonts() {
			FT_Error err = FT_Init_FreeType(&library);
			if (err) {
				std::cerr << "Error initializing FT Library!\nError code: " << err << '\n' << std::endl;
			} else {
				initialized = true;
				FT_Library_Version(library, &version_major, &version_minor, &version_patch);
				std::cout << "Done initializing FreeType Font Library!\n"
					<< "Library version: " << version_major << '.' << version_minor << '.' << version_patch << '\n' << std::endl;
			}
		}

		FTFonts::~FTFonts() {
			if (unload_count) {
				std::sort(unloaded, unloaded + unload_count - 1);
				size_t unload_offset = 0;
				for (size_t i = 0; i < face_count; i++) {
					if (unload_offset < unload_count && i == unloaded[unload_offset]) unload_offset++;
					else {
						FT_Error err = FT_Done_Face(faces[i]);
						if (err) {
							std::cerr << "Error unloading font face!\nError code: " << err << '\n' << std::endl;
						}
					}
				}
			} else {
				for (size_t i = 0; i < face_count; i++) {
					FT_Error err = FT_Done_Face(faces[i]);
					if (err) {
						std::cerr << "Error unloading font face!\nError code: " << err << '\n' << std::endl;
					}
				}
			}
			FT_Error err = FT_Done_FreeType(library);
			if (err) {
				std::cerr << "Error unloading FreeType Library!\nError code: " << err << '\n' << std::endl;
			}
			delete[] unloaded;
			delete[] faces;
		}

		size_t FTFonts::load_face(const FT_Byte* data, size_t size, size_t index) {
			if (unload_count) {
				size_t rtn = unloaded[unload_count - 1];
				FT_Error err = FT_New_Memory_Face(library, data, FT_Long(size), FT_Long(index), &faces[rtn]);
				if (err) {
					std::cerr << "Error loading font face from memory!\nError code: " << err << '\n' << std::endl;
					return -1;
				}
				unload_count--;
				return rtn;
			}
			if (face_cache == face_count) {
				if (!resize_faces()) {
					return -1;
				}
			}
			size_t rtn = face_count;
			FT_Error err = FT_New_Memory_Face(library, data, FT_Long(size), FT_Long(index), &faces[rtn]);
			if (err) {
				std::cerr << "Error loading font face from memory!\nError code: " << err << '\n' << std::endl;
				return -1;
			}
			face_count++;
			return rtn;
		}

		bool Graphics::FTFonts::unload_face(const size_t index) {
			if (index >= face_count) return false;
			try {
				if (unload_count == unload_cache) {
					size_t new_unload_cache = size_t(unload_cache * 1.618);
					new_unload_cache++;
					size_t* new_buffer = new size_t[new_unload_cache];
					for (size_t i = 0; i < unload_count; i++) {
						new_buffer[i] = unloaded[i];
					}
					std::swap(unloaded, new_buffer);
					unload_cache = new_unload_cache;
					delete[] new_buffer;
				}
			} catch (...) {
				return false;
			}
			FT_Error err = FT_Done_Face(faces[index]);
			if (err) {
				std::cerr << "Error unloading font!\n" << "Error code: " << err << '\n' << std::endl;
				return false;
			}
			unloaded[unload_count] = index;
			unload_count++;
			return true;
		}

		bool FTFonts::resize_faces() {
			try {
				size_t new_face_cache = size_t(face_cache * 1.618);
				new_face_cache++;
				FT_Face* new_buffer = new FT_Face[new_face_cache];
				for (size_t i = 0; i < face_count; i++) {
					new_buffer[i] = faces[i];
				}
				std::swap(faces, new_buffer);
				face_cache = new_face_cache;
				delete[] new_buffer;
				return true;
			} catch (...) {
				return false;
			}
		}

		bool FTFonts::reserve_faces(size_t size) {
			if (size < face_count) return false;
			try {
				FT_Face* new_buffer = new FT_Face[size];
				face_cache = size;
				for (size_t i = 0; i < face_count; i++) {
					new_buffer[i] = faces[i];
				}
				std::swap(faces, new_buffer);
				delete[] new_buffer;
				return true;
			} catch (...) {
				return false;
			}
		}

		FT_Face& FTFonts::get_face(size_t index) {
			if (index >= face_count) {
				std::cerr << "Error getting face: index overflow!\nUsing default!\n" << std::endl;
				return faces[0];
			}
			return faces[index];
		}
	}
}