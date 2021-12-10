#include "g.h"
#include "state.hpp"
#include "renderer.hpp"
#include "mechanics.hpp"

struct unnatural_selection : public g::core
{

g::asset::store assets;
us::renderer renderer;

us::state state;

unnatural_selection() = default;
~unnatural_selection() = default;

virtual bool initialize()
{
	// state.level = std::make_shared<us::level>(assets.tex("9x9.png"));
	state.level = std::make_shared<us::level>(assets.tex("Level.png"));

	std::cerr << state.level->info() << std::endl;

	std::cerr << "Distances" << std::endl;
	auto& spawn = state.level->spawn_points[0];
	auto& distances = state.level->cells[spawn[0]][spawn[1]].node_distances;

	state.player.position = { (float)state.level->lymph_nodes[0][0], 2.f, (float)state.level->lymph_nodes[0][1] };

	glDisable(GL_CULL_FACE);
	glPointSize(4);

	for (auto kvp : distances)
	{
		std::cerr << "node_id: " << kvp.first << " dist: " << distances[kvp.first] << std::endl;
	}

	for (auto& spawn : state.level->spawn_points)
	{
		for (unsigned i = 100; i--;)
		{
			us::baddie baddie;
			baddie.position = {(float)spawn[0] + us::randf(), 1.f + us::randf(), (float)spawn[1] + us::randf()};
			baddie.genes.damage = 1;
			baddie.genes.speed = 1 + rand() % 9;
			baddie.genes.target_node = rand() % state.level->living_lymph_nodes().size();
			baddie.hp = 1;
			state.baddies.push_back(baddie);
		}
	}

	glfwSetInputMode(g::gfx::GLFW_WIN, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return true;
}


void spawn_projectile(us::state& state, const vec<3>& position, const vec<3>& velocity)
{
	us::projectile projectile;
	projectile.position = position;
	projectile.velocity = velocity;
	state.projectiles.push_back(projectile);
}


virtual void update(float dt)
{
    us::update_projectiles(state, dt);
    us::update_baddies(state, dt);
    us::update_player(state, dt);

    // state.player.update(dt, *state.level);
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
    	exit(0);
    }

    state.player.orientation = state.player.get_orientation();

	renderer.draw(assets, state);

 	state.time += dt;
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