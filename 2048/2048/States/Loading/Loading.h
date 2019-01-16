#pragma once

#include "..\..\Game_2048.h"

#include <Graphics\Models\Text\Text.h>

namespace States {
	class Loading:
		public Engine::Core::State{
	public:
		Loading(Engine::Core::GlobalManagers& given_managers);
		const char* text = "Loading...";
		uint32_t* text_code = nullptr;
		void Destroy() override;
		void Draw() override;
		void Update() override;//put loading and loading thread here...
		void Loop() override;
		void notifyContextColse(size_t ID) {
			running = false;
		}
	};
}
