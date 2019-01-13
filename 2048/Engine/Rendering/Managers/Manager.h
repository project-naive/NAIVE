#pragma once

namespace Engine {
	namespace Rendering {
		class Manager {
			virtual void Init() = 0;
			virtual void Destroy() = 0;
		};
	}
}
