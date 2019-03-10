#pragma once

#include <Core\Game.h>

class Game_2048:
	public Engine::Core::Game {
public:
	Game_2048(Engine::Graphics::WindowInfo& info);
	~Game_2048();
	void notifyRefresh(size_t contextID) override final {
		state->Refresh(contextID);
//		Managers.ContextManager.SwapBuffers();
	}
	void MainLoop() override final;
	void notifyFBSize(int width, int height, size_t contextID = 0) override final {
		Managers.ContextManager.Resize(width, height, contextID);
		state->Refresh(contextID);
//		Managers.ContextManager.SwapBuffers();
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
	void notifyKey(int key, int scancode, int action, int mods, size_t contextID = 0) override final {
		state->notifyKey(key, scancode, action, mods, contextID);
	};
};
