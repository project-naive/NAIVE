#pragma once

#include "..\Shaders\Shader.h"

namespace Rendering {
	class Model {
	public:
		GLuint shader = 0;
		GLuint vao = 0;
		GLuint* vbos = nullptr;
		GLuint vbo_count = 0;
		virtual void Draw() = 0;
		~Model() {
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(vbo_count, vbos);
			delete[] vbos;
		}
	};
}


