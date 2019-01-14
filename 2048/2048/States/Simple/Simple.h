#pragma once

#include "..\..\..\Engine\Core\State.h"
#include "..\..\..\Engine\Graphics\Models\Triangle\Triangle.h"

#include "..\..\Game_2048.h"

namespace States {
	class Simple:
		public Engine::Core::State{
	public:
		Simple(Engine::Core::GlobalManagers& given_managers);
		~Simple();
		Engine::Graphics::Models::Triangle* TriangleModel;

		void Draw() override;
		void Update() override;
		void Loop() override;
	};
}
