#include "g.h"
#include "state.hpp"
#include "renderer.hpp"
#include "mechanics.hpp"

struct unnatural_selection : public g::core
{

g::asset::store assets;
us::renderer renderer;

us::state state;

int frame = 0;
int quit_frame = 0;

unnatural_selection() = default;
~unnatural_selection() = default;

virtual bool initialize()
{
	// state.level = std::make_shared<us::level>(assets.tex("9x9.png"));
	state.level = std::make_shared<us::level>(assets.tex("Level_1.png"));

	std::cerr << state.level->info() << std::endl;

	std::cerr << "Distances" << std::endl;
	auto& spawn = state.level->spawn_points[0];
	auto& distances = state.level->cells[spawn[0]][spawn[1]].node_distances;

	state.level->for_each_neighbor(state.level->lymph_nodes[0][0], state.level->lymph_nodes[0][1], [&](us::level::cell& cell, int r, int c) {
		if (cell.is_floor)
		{
			state.player.position = { (float)r, 2.f, (float)c };
			return true;
		}

		return false;
	});

	glDisable(GL_CULL_FACE);

	for (auto kvp : distances)
	{
		std::cerr << "node_id: " << kvp.first << " dist: " << distances[kvp.first] << std::endl;
	}

	for (unsigned i = 100; i--;)
	{
		us::baddie baddie;
		baddie.initialize();
		state.next_generation.push_back(baddie);
	}

	for (unsigned i = 0; i < 8; i++)
	{
		state.impacts[i] = g::snd::source_ring(&assets.sound("bullet_impact_flesh_" + std::to_string(i + 1) + ".wav"), 3);
	}

	state.gun_sounds[0] = g::snd::source_ring(&assets.sound("sound_carbine.wav"), 10);
	state.gun_sounds[1] = g::snd::source_ring(&assets.sound("LAZER_GUN_1.wav"), 10);
	state.gun_sounds[2] = g::snd::source_ring(&assets.sound("sound_shotty.wav"), 10);


	state.wall_impacts[0] = g::snd::source_ring(&assets.sound("fleshy-impact-miss-wall-hit.wav"), 3);
	state.wall_impacts[1] = g::snd::source_ring(&assets.sound("fleshywallimpact.wav"), 3);


	state.virus_sounds = g::snd::source_ring(&assets.sound("viral_sound_1.wav"), 20);
	state.ambient = g::snd::source(&assets.sound("ambient-liquid.looping.wav"));
	state.ambient.position({state.level->width() / 2.f, 0, state.level->height() / 2.f});

	state.wave_start = g::snd::source(&assets.sound("Wave_Start.wav"));

	state.ambient.play();

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
	if (state.onboarding_done)
	{
		us::update_wave(state, dt);
	}

    us::update_baddies(state, dt);
    us::update_projectiles(state, dt);
    us::update_player(state, dt);

    // state.player.update(dt, *state.level);
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
    	exit(0);
    }

    static bool e_was_down;
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_E) == GLFW_PRESS)
    {
    	if (!e_was_down)
    	{
    		state.onboarding_step++;
    		e_was_down = true;
    	}
    }
    else
    {
    	e_was_down = false;
    }

	auto mode = glfwGetInputMode(g::gfx::GLFW_WIN, GLFW_CURSOR);
	if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_ESCAPE) == GLFW_PRESS && frame - quit_frame > 100)
	{
		switch(mode)
		{
			case GLFW_CURSOR_NORMAL:
				glfwSetWindowShouldClose(g::gfx::GLFW_WIN, GLFW_TRUE);
				break;
			case GLFW_CURSOR_DISABLED:
				glfwSetInputMode(g::gfx::GLFW_WIN, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				break;
		}

		quit_frame = frame;
	}

    state.player.orientation = state.player.get_orientation();

	renderer.draw(assets, state);

 	state.time += dt;
 	frame++;

    state.ambient.update();
    state.virus_sounds.update();
	
	for (unsigned i = 0; i < 2; i++)    
    state.wall_impacts[i].update();

    for (unsigned i = 0; i < 8; i++)
    state.impacts[i].update();

    for (unsigned i = 0; i < 3; i++)
    state.gun_sounds[i].update();
}

};


#include <emscripten.h>

EM_JS(int, canvas_get_width, (), {
  return document.getElementById('canvas').width;
});

EM_JS(int, canvas_get_height, (), {
  return document.getElementById('canvas').height;
});

int main (int argc, const char* argv[])
{
	unnatural_selection game;

	g::core::opts opts;

	opts.name = "unnatural selection";
	opts.gfx.fullscreen = false;

	auto monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	// glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	// glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	// glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	// glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);					
	// g::gfx::GLFW_WIN = glfwCreateWindow(mode->width, mode->height, opts.name ? opts.name : "", monitor, NULL);

	opts.gfx.width = canvas_get_width();
	opts.gfx.height = canvas_get_height();

	game.start(opts);

	return 0;
}