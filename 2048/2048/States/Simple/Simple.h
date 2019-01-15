#pragma once

#include "..\..\Game_2048.h"

#include "..\..\..\Engine\Graphics\Models\Text\Text.h"
#include "..\..\..\Engine\Graphics\Models\Triangle\Triangle.h"

namespace States {
	class Simple:
		public Engine::Core::State{
	public:
		Simple(Engine::Core::GlobalManagers& given_managers);
		~Simple();
		Engine::Graphics::Models::Triangle* TriangleModel;
		Engine::Graphics::Models::Text* TextRenderer;

		void Draw() override;
		void Update() override;
		void Loop() override;
	};
}
