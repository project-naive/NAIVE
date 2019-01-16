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

			size_t id;
		public:
			State(GlobalManagers& given_managers):
				Managers(given_managers) {
				start_time = std::chrono::steady_clock::now();
				last_update_time = std::chrono::steady_clock::now();
			}
			GlobalManagers& Managers;
			~State() {};

			bool running = true;
			virtual void Destroy() {};
			virtual void Loop() = 0;

			virtual void Draw() = 0;
			virtual void Update() = 0;

			virtual void notifyContextClose(size_t ID) {};
		};
	}
}