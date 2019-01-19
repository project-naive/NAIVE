#pragma once

#include <Core\Game.h>


class Game_2048:
	public Engine::Core::Game {
public:
	Game_2048(Engine::Graphics::WindowInfo& info);
	~Game_2048();
	void notifyRefresh(size_t contextID) override final {
		Managers.ContextManager.Refresh();
	}
	void MainLoop() override final {
		glfwPollEvents();
		while(running){
			state->Loop();
			running &= state->running;
		}
	}
	void notifyFBSize(int width, int height, size_t contextID = 0) override final {
		Managers.ContextManager.Resize(width, height, contextID);
//		Managers.ContextManager.Refresh();
//		state->Draw();
	};
	virtual void notifyClose(size_t contextID = 0) override final {
		if (!state->notifyContextClose(contextID)) {
			Managers.ContextManager.DelContext(contextID);
			if (Managers.ContextManager.noContext()) {
				running = false;
			}
		}
	}
	void notifyFocus(int flag, size_t contextID = 0) override final {};
	void notifyIconify(int flag, size_t contextID = 0) override final {};
	void notifyPos(int width, int height, size_t contextID = 0) override final {};
	void notifySize(int width, int height, size_t contextID = 0) override final {};
};
