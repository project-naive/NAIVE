#pragma once

#include <GL\glew.h>

namespace Engine {
	namespace Graphics {
		namespace Managers{
			class Shader;
		}
		class Model {
		public:
			const size_t shader;
			GLuint vao = 0;
			GLuint* vbos = nullptr;
			GLuint vbo_count = 0;
			virtual GLuint Begin() = 0;
			virtual void Draw() = 0;
			virtual void Update() = 0;
			~Model() {
				glDeleteVertexArrays(1, &vao);
				glDeleteBuffers(vbo_count, vbos);
				delete[] vbos;
			}
			Model(Managers::Shader& given_manager, size_t shaderID):
				manager(given_manager),
				shader(shaderID) {};
			Managers::Shader& manager;
		};
	}
}

