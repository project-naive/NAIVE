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
			GLuint shader;
			GLuint vao = 0;
			GLuint* vbos = nullptr;
			GLuint vbo_count = 0;
			void Begin() {
				glBindVertexArray(vao);
				glUseProgram(shader);
			};
			virtual void Draw() = 0;
			virtual void Update()=0;
			~Model() {
				glDeleteVertexArrays(1, &vao);
				glDeleteBuffers(vbo_count, vbos);
				delete[] vbos;
			}
			Model(const Managers::Shader& given_manager):manager(given_manager) {};
			const Managers::Shader& manager;
		};
	}
}

