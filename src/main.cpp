#include "g.h"
#include "state.hpp"
#include "renderer.hpp"

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

	return true;
}


virtual void update(float dt)
{
 	vec<2> dir = {};

 	state.time += dt;

    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_W) == GLFW_PRESS) dir += { 0, dt};
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_S) == GLFW_PRESS) dir += { 0,-dt};
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_A) == GLFW_PRESS) dir += {-dt, 0};
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_D) == GLFW_PRESS) dir += { dt, 0};
    // if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_Q) == GLFW_PRESS) state.player.d_roll(-dt);
    // if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_E) == GLFW_PRESS) state.player.d_roll(dt);
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_LEFT) == GLFW_PRESS) state.player.theta += (-dt);
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_RIGHT) == GLFW_PRESS) state.player.theta += (dt);
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_UP) == GLFW_PRESS) state.player.phi += (dt);
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_DOWN) == GLFW_PRESS) state.player.phi += (-dt);
    
    state.player.walk(dir * PLAYER_SPEED);
    state.player.update(dt, *state.level);

    state.player.orientation = state.player.get_orientation();

    // update particles
    for (auto& projectile : state.projectiles)
    {
    	projectile.position += projectile.velocity * dt;
    }

	renderer.draw(assets, state);
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