#include "Loading.h"

#include <iostream>
#include <fstream>

namespace States {
	Loading::Loading(Engine::Core::GlobalManagers& given_managers):
		State(given_managers){
		text_code = new uint32_t[strlen(text) + 1];
		for (size_t i = 0; i < strlen(text); i++) {
			text_code[i] = text[i];
		}
	};

	void Loading::Destroy(){
		delete[] text_code;
	}

	void Loading::Draw() {
		Managers.TextManager.renderText(text_code, glm::vec2(20.0f, 20.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, strlen(text));
		Managers.ContextManager.Refresh();
	}
	void Loading::Update(){}
	void Loading::Loop(){
		while(running){
			Update();
			Draw();
			glfwWaitEvents();
		}
	}
}
