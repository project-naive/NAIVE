#pragma once

#include "..\Shaders\Shader.h"
#include "..\..\Core\State.h"

namespace Engine {
	namespace Graphics {
		class Model {
		public:
			GLuint shader = 0;
			GLuint vao = 0;
			GLuint* vbos = nullptr;
			GLuint vbo_count = 0;
			virtual void Draw() = 0;
			virtual void Update()=0;
			~Model() {
				glDeleteVertexArrays(1, &vao);
				glDeleteBuffers(vbo_count, vbos);
				delete[] vbos;
			}
			Model(Core::State* given_parent):parent(given_parent) {};
			const Core::State* parent;
		};
	}
}

