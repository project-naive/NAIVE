#pragma once

#include "..\..\..\Engine\Core\State.h"
#include "..\..\..\Engine\Rendering\Models\Triangle\Triangle.h"

#include "..\..\Game_2048.h"

namespace States {
	class Simple:
		public Engine::Core::State{
	public:
		Simple(Game_2048* parent);
		~Simple();
		Engine::Rendering::Models::Triangle* TriangleModel;
		void Init() override;
		void Destroy() override;

		void Draw() override;
		void Update() override;
		void Loop() override;
	};
}
