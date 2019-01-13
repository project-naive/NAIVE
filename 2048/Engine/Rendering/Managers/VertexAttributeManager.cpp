#include "VertexAttributeManager.h"

#include <iostream>

namespace Rendering {
	namespace Manager {
		GLuint VertexAttribute::used_count = 0;
		GLuint* VertexAttribute::unloaded = nullptr;
		GLuint VertexAttribute::unload_count = 0;
		GLuint VertexAttribute::unload_cache = 0;

		GLuint VertexAttribute::RequestAttribute() {
			if (unload_count) {
				GLuint rtn = unloaded[unload_count - 1];
				glEnableVertexAttribArray(rtn);
				unload_count--;
				return rtn;
			}
			if (used_count >= GL_MAX_VERTEX_ATTRIBS) {
				return -1;
			}
			glEnableVertexAttribArray(used_count);
			used_count++;
			return used_count - 1;
		}

		bool VertexAttribute::RemoveAttribute(GLuint index) {
			if (unload_count == unload_cache) {
				GLuint* new_buffer = nullptr;
				try {
					GLuint new_cache = GLuint(unload_cache*1.618);
					new_cache++;
					new_buffer = new GLuint[new_cache];
					for (GLuint i = 0; i < unload_count; i++) {
						new_buffer[i] = unloaded[i];
					}
					std::swap(unloaded, new_buffer);
					delete[] new_buffer;
				} catch (...) {
					delete[] new_buffer;
					return false;
				}
			}
			unloaded[unload_count] = index;
			unload_count++;
			glDisableVertexAttribArray(index);
			return true;
		}
	}
}


