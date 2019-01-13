//The class for a given state in game

#pragma once

#include <chrono>

namespace Engine {
	namespace Core {
		class Game;
		class State {
		public:
			Game* parent = nullptr;
			virtual void Init() = 0;
			virtual void Destroy() = 0;

			virtual void Draw() = 0;
			virtual void Update() = 0;
			virtual void Loop() = 0;

			std::chrono::steady_clock::time_point start_time;
			std::chrono::high_resolution_clock::time_point last_update_time;

			size_t id;
		};
	}
}