#pragma once

#include "..\..\Test.h"


namespace States {
	class Playground:
		public Engine::Core::State {
	public:
		Playground(Engine::Core::GlobalManagers& given_managers);;
		void Destroy() override final {};
		void Draw() override final {};
		void Update() override final {};
		void Loop() override final;;
		void Refresh(size_t ID = 0) override final {}
		bool notifyContextClose(size_t ID) override final;
		void notifyKey(int key, int scancode, int action, int mods, size_t contextID = 0) override final {}
	};
}
