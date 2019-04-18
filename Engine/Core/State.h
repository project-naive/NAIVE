//The class for a given state in game

#pragma once

#include <chrono>

namespace Engine {
	namespace Core {
		class Game;
		class GlobalManagers;
		class State {
		protected:
			std::chrono::steady_clock::time_point start_time;
			std::chrono::high_resolution_clock::time_point last_update_time;

			size_t id = 0;
		public:
			State(GlobalManagers& given_managers):
				Managers(given_managers) {
				start_time = std::chrono::steady_clock::now();
				last_update_time = std::chrono::steady_clock::now();
			}
			GlobalManagers& Managers;
			~State() = default;

			bool running = true;
			virtual void Destroy() = 0;
			virtual void Loop() = 0;

			virtual void Draw() = 0;
			virtual void Update() = 0;

			virtual bool notifyContextClose(size_t ID) = 0;
			virtual void Refresh(size_t ID = 0) = 0;
			virtual void notifyKey(int key, int scancode, int action, int mods, size_t contextID = 0) = 0;
			virtual void notifyFBSize(int width, int height, size_t contextID = 0) = 0;
		private:
			size_t* ContextIDs = nullptr;
			size_t context_count = 0;
		};
	}
}