#include "g.h"
#include "state.hpp"


struct unnatural_selection : public g::core
{

g::asset::store assets;
us::state state;

unnatural_selection() = default;
~unnatural_selection() = default;

virtual bool initialize()
{
	state.level = std::make_shared<us::level>(assets.tex("Level.png"));

	std::cerr << state.level->info() << std::endl;

	std::cerr << "Distances" << std::endl;
	auto& spawn = state.level->spawn_points[0];
	auto& distances = state.level->cells[spawn[0]][spawn[1]].node_distances;

	for (auto kvp : distances)
	{
		std::cerr << "node_id: " << kvp.first << " dist: " << distances[kvp.first] << std::endl;
	}

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