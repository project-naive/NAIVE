#include "WebMPlayer.h"

int main() {
	Engine::Graphics::WindowInfo info{ Engine::Graphics::Contexts::Default::default_window() };
	info.Basic_Info.width = 800;
	info.Basic_Info.height = 600;

	WebMPlayer player(info);
}




