/*****************************************************************
*	This section defines a class that manages shader programs.
*	It is also responsible for calling the OpenGL APIs when needed.
*
*
*/

#pragma once

#include <GL\glew.h>
#include <glfw\glfw3.h>

namespace Rendering {
	namespace Manager {
		class Shader {
			GLuint program;
			GLuint CreateProgram();
		};
	}
}

