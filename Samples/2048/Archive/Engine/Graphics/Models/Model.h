#pragma once

#include "..\Shaders\Shader.h"
#include "..\..\Core\State.h"

namespace Engine {
	namespace Graphics {
		namespace Managers{
			class Shader;
		}
		class Model {
		public:
			size_t shader = 0;
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
			Model(Managers::Shader& given_manager):manager(given_manager) {};
			Managers::Shader& manager;
		};
	}
}

