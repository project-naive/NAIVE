#pragma once

#include "..\..\Core\Game.h"

namespace Engine {
	namespace Graphics {
		namespace Containers {
			class ContextContainer {
			public:
				ContextContainer() = delete;
				ContextContainer(
					Core::GlobalManagers& GlobalManagers, 
					const size_t contextID);
				const size_t ID;
			private:
				Core::GlobalManagers& Managers;
				size_t ActiveProgram;
				size_t ActiveFBO;
				GLuint ActiveVAO;

				size_t* FBOs;
				size_t FBO_count;
			};
		}
	}
}

