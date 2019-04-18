#pragma once

#include <Core/Game.h>

#include <vpx/vpx_decoder.h>

#include "..\Threading\ThreadPool\ThreadPool.h"

#include "..\..\Samples\2048\2048\States\Simple\Models\TextureQuad\TextureQuad.h"

class SimplePlay 
	:public Engine::Core::State {
public:
	SimplePlay(Engine::Core::GlobalManagers& given_managers);
	~SimplePlay();
	void Destroy() override final {};
	void Draw() override final {};
	void Update() override final {};
	void Loop() override final;
	void Refresh(size_t ID = 0) override final;
	bool notifyContextClose(size_t ID) override final {
		running = false;
		return false;
	};
	void notifyKey(int key, int scancode, int action, int mods, size_t contextID = 0) override final {}
	ThreadPool& thread_pool;
	char* buffer[2];
	std::atomic_flag use_state[2] = { ATOMIC_FLAG_INIT, ATOMIC_FLAG_INIT };
	std::atomic<bool> load_state[2] = { false, false };
	std::atomic<bool> done = false;
	uint32_t buff_size[2]{};
	uint32_t frame_num[2]{};
	short width = 0;
	short height = 0;
	uint32_t fourcc = 0;
	std::atomic<bool> all_done = false;
	std::atomic<bool> decode_done = false;
	std::atomic<bool> buff_decode[2] = { false, false };
	GLuint  pixbuffs[6];
	GLuint* pixelbuffers[2]{ pixbuffs, pixbuffs + 3 };
	int wy; int wuv; int hy; int huv;
	State_2048::Models::TextureQuad* tex_model = nullptr;
	GLuint textures[2];
};

class WebMPlayer : public Engine::Core::Game {
public:
	WebMPlayer() = delete;
	WebMPlayer(Engine::Graphics::WindowInfo& info);
	~WebMPlayer();
	void notifyRefresh(size_t contextID) override final {
		state->Refresh(contextID);
	}
	void MainLoop() override final;
	void notifyFBSize(int width, int height, size_t contextID = 0) override final {};
	void notifyClose(size_t contextID = 0) override final {
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
	void notifyKey(int key, int scancode, int action, int mods, size_t contextID = 0) override final {};
};
