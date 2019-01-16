#pragma once

#include "..\..\Game_2048.h"

namespace States {
	class State_2048:
		public Engine::Core::State{
	public:
		State_2048(Engine::Core::GlobalManagers& given_managers);
		void Destroy() override;
		void Draw() override;
		void Update() override;
		void Loop() override;
	};
}