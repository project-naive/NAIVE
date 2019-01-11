#pragma once

namespace Global {
	extern int argc_m;
	extern char** argv_m;
	extern char** env_m;

	extern const char* game_title;

	extern bool idle;

	void Init();
	void CleanUp();

	namespace Callback {
		void error(int err_code, const char* info);
	}
}

