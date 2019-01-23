#include "Test.h"

#include "States/Playground/Playground.h"

Test::Test(Engine::Graphics::WindowInfo & info): Game(info) {
	state = new States::Playground(Managers);

}

Test::~Test() {


}
