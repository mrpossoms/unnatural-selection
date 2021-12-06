#include "g.h"
#include "state.hpp"


struct unnatural_selection : public g::core
{

us::state state;

unnatural_selection() = default;
~unnatural_selection() = default;

virtual bool initialize()
{
	return true;
}


virtual void update(float dt)
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

};


int main (int argc, const char* argv[])
{
	unnatural_selection game;

	game.start({
		"unnatural selection", true, 1024, 768
	});

	return 0;
}