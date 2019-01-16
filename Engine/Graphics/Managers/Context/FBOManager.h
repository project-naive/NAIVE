#pragma once

#include "..\Globals\ContextManager.h"

namespace Engine {
	namespace Graphics {
		namespace Managers {
			class FBO {
			public:
				FBO() = delete;
				FBO(const Context& ContextManager): 
					context_ID(ContextManager.GetCurrent()){}
			private:
				const size_t context_ID;
			};
		}
	}
}