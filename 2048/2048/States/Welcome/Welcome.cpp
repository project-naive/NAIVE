#include "Welcome.h"

namespace States {
	Welcome::Welcome(Engine::Core::GlobalManagers & given_managers):
		State(given_managers) {
		size_t width, height;
		Managers.ContextManager.GetCurrentDefaultResolution(width, height);
		const char* message1 = "2048 Game";
		const char* message2 = "Press Space Bar to start!";
		const float scale1 = 1.0f;
		const float scale2 = 0.5f;
		const float division = 64.0f;
		float TextHeight = Managers.TextManager.GetHeight();
		float length1 = Managers.TextManager.TextLength(message1);
		float length2 = Managers.TextManager.TextLength(message2);
		float x0 = width / 2.0f - ( length1 / 2.0f ) * scale1;
		float y1 = height / 2.0f - ( TextHeight*( scale1 + scale2 ) + division ) / 2.0f;
		float x1 = width / 2.0f - ( length2 / 2.0f )*scale2;
		float y0 = y1 + scale2 * TextHeight + division;
		glClearColor(0xF3 / 255.0f, 0xE2 / 255.0f, 0xA9 / 255.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Managers.TextManager.renderText(message1, glm::vec2(x0, y0), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), scale1);
		Managers.TextManager.renderText(message2, glm::vec2(x1, y1), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), scale2);
		glFlush();
	}

	void Welcome::Loop() {
		while (running) {
			glfwWaitEvents();
			if (ready) {
				running = false;
			}
		}
	}
}


