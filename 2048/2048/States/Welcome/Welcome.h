#pragma once

#include "..\..\Game_2048.h"


namespace States {
	class Welcome:
		public Engine::Core::State {
	public:
		Welcome(Engine::Core::GlobalManagers& given_managers);
		void Destroy() override final {};
		void Draw() override final {};
		void Update() override final {};
		void Loop() override final;
		void Refresh(size_t ID = 0) override final;
		bool notifyContextClose(size_t ID) override final {
			running = false;
			return false;
		};
		void notifyKey(int key, int scancode, int action, int mods, size_t contextID = 0) override final {
			if (action == GLFW_PRESS) {
				switch (key) {
					case GLFW_KEY_SPACE:
						ready = true;
				}
			}
		}
		bool ready = false;
	};
}
