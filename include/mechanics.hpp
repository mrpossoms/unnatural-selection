#pragma once

#include "state.hpp"

namespace us
{

static void update_projectiles(us::state& state, float dt)
{
    // update projectiles
	for (unsigned i = 0; i < state.projectiles.size(); i++)
	{
		us::projectile& projectile = state.projectiles[i];
		auto new_pos = projectile.position + projectile.velocity * dt;

	    auto hit_level = !state.level->get_cell(new_pos).is_floor || new_pos[1] <= 0 || new_pos[1] >= 6;

	    if (hit_level)
	    {
	    	auto i = rand() % 2;
	    	state.wall_impacts[i].position(projectile.position);
	    	state.wall_impacts[i].play();
	    }

		auto is_dead = hit_level || projectile.life <= 0;
		if (is_dead)
		{
			state.projectiles.remove_at(i);
			continue;
		}

		// deal damage to baddies
		for (unsigned j = 0; j < state.baddies.size(); j++)
		{
			auto& baddie = state.baddies[j];
			
			if (baddie.hp <= 0) { continue; }

			auto t = intersect::ray_sphere(projectile.position, projectile.velocity, baddie.position, 0.5f);

			if (t <= dt)
			{
		    	auto i = rand() % 8;
		    	state.impacts[i].position(projectile.position);
		    	state.impacts[i].play();

				for (unsigned i = 6; i--;)
				state.particles.spawn(baddie.position, vec<3>{randf(), randf(), randf()} * 4, state.time, state.time + 10, {(rand() % 11)/11.f});

				projectile.life = 0;
				// baddie.hp -= 1;
				baddie.take_hit(projectile);
				break;
			}
		}

		projectile.position = new_pos;
		projectile.life -= dt;
	}
}


static void update_baddies(us::state& state, float dt)
{
	auto living_lymph_nodes = state.level->living_lymph_nodes();

    // update baddies
	for (unsigned i = 0; i < state.baddies.size(); i++)
	{
		auto& baddie = state.baddies[i];
		int r = baddie.position[0]; int c = baddie.position[2];
		// auto is_dead = !state.level->cells[(int)(new_pos[0] + 0.5)][(int)(new_pos[2] + 0.5)].is_floor ||
		//                new_pos[1] <= 0 || new_pos[1] >= 6 ||
		//                projectile.life <= 0;

		if (baddie.hp <= 0)
		{
			// state.baddies.remove_at(i);
			continue;
		}

		auto& my_cell = state.level->get_cell(baddie.position);
		if (my_cell.lymph_node_hp > 0)
		{
			my_cell.lymph_node_hp -= baddie.damage;
			baddie.damage_dealt += baddie.damage;
			baddie.hp = 0; // die on collision
		}

		if (rand() % 100 == 0)
		{
			state.virus_sounds.position(baddie.position);
			state.virus_sounds.play();
		}

		if (living_lymph_nodes.size() > 0)
		{ // do path finding

			level::cell* best_node = nullptr;
			float best_dist = 10e6;
			auto target_node_index = living_lymph_nodes[baddie.genes.target_node % living_lymph_nodes.size()];

			// look at neighboring nav nodes for the cell with the smallest distance to the target
			state.level->for_each_neighbor(r, c, [&](level::cell& neighbor, unsigned nr, unsigned nc) -> bool {
				auto itr = neighbor.node_distances.find(target_node_index);
				if (neighbor.is_floor && itr != neighbor.node_distances.end())
				{
					if (itr->second < best_dist)
					{
						best_node = &neighbor;
						best_dist = itr->second;
					}
				}
			});

			if (best_node)
			{
				baddie.velocity += (vec<3>{best_node->r + 0.5f, randf() + (i % 6), best_node->c + 0.5f} - baddie.position) * 0.1 * baddie.speed;
				baddie.progress += baddie.velocity.magnitude() * dt;
			}
		}

		auto new_pos = baddie.position + baddie.velocity * dt;
		baddie.position = new_pos;
		baddie.velocity *= 0.9f;
		// projectile.life -= dt;
	}	
}

void update_player(us::state& state, float dt)
{
	auto& player = state.player;
	auto new_pos = player.position + player.velocity * dt;
	
	player.is_sprinting = false;

 	vec<2> dir = {};
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_W) == GLFW_PRESS) dir += { 0, dt};
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_S) == GLFW_PRESS) dir += { 0,-dt};
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_A) == GLFW_PRESS) dir += {-dt, 0};
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_D) == GLFW_PRESS) dir += { dt, 0};
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_LEFT) == GLFW_PRESS) player.theta += (-dt);
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_RIGHT) == GLFW_PRESS) player.theta += (dt);
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_UP) == GLFW_PRESS) player.phi += (dt);
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_DOWN) == GLFW_PRESS) player.phi += (-dt);
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_DOWN) == GLFW_PRESS) player.phi += (-dt);
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) player.is_sprinting = true;

	static double xlast, ylast;
	double xpos = 0, ypos = 0;
	auto mode = glfwGetInputMode(g::gfx::GLFW_WIN, GLFW_CURSOR);

	if (GLFW_CURSOR_DISABLED == mode)
	{
		glfwGetCursorPos(g::gfx::GLFW_WIN, &xpos, &ypos);
	}

	auto dx = xpos - xlast;
	auto dy = ypos - ylast;
	player.phi += (-dy * dt);
	player.theta += (dx * dt);
	xlast = xpos; ylast = ypos;

	if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_1) == GLFW_PRESS) player.selected_weapon = 0;
	if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_2) == GLFW_PRESS) player.selected_weapon = 1;
	if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_3) == GLFW_PRESS) player.selected_weapon = 2;

    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_SPACE) == GLFW_PRESS ||
    	glfwGetMouseButton(g::gfx::GLFW_WIN, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
			if (player.cool_down <= 0)
			{
				auto& selected_weapon = player.selected_weapon;
				const auto spread = player.weapon_spreads[selected_weapon];

				for (unsigned i = 0; i < player.weapon_projectiles[selected_weapon]; i++)
				{
					us::projectile p;
					p.type = (us::projectile::category)selected_weapon;
					p.position = player.position + player.forward() * 0.25f - player.up() * 0.1;
					p.velocity = player.forward() * player.weapon_velocities[selected_weapon] + (player.up() * randf() * spread) + (player.left() * randf() * spread);
					p.life = 10;
					state.projectiles.push_back(p);
				}

				player.cool_down = player.weapon_cool_downs[selected_weapon];
			}
    	// }
    }

    state.player.walk(dir * (player.speed * (player.is_sprinting ? 2.f : 1.f)));

	if (state.level->get_cell(new_pos + vec<3>{0.5f, 0.5f, 0.5f}).is_floor)
	{
		player.position = new_pos;	
	}

	player.cool_down = std::max<float>(0, player.cool_down - dt);

	// position += velocity * dt;
	player.velocity *= 0.9f;

	player.phi = std::max<float>(-M_PI / 4, player.phi);
	player.phi = std::min<float>( M_PI / 4, player.phi);

	g::snd::set_observer(player.position, player.velocity, player.orientation);
}

void update_wave(us::state& state, float dt)
{
	if (state.level->living_lymph_nodes().size() <= 0) { return; }

	if (state.wave.count_down <= 0)
	{
		if (state.baddies.size() > 0)
		{
			g::ai::evolution::generation<us::baddie>(state.baddies, state.next_generation, {});
			// state.baddies.clear(); // get rid of old baddies

			std::cerr << "TOP PERFORMERS" << std::endl;
			for (unsigned i = 0; i < std::min<unsigned>(10, state.baddies.size()); i++)
			{
				std::cerr << std::to_string(i) << ": " << state.baddies[i].score() << std::endl;
			}
		}



		state.wave.spawn_point = state.level->spawn_points[rand() % state.level->spawn_points.size()];
		state.wave.baddies_to_spawn = WAVE_BASE_ENEMY_COUNT + state.wave.number * (WAVE_BASE_ENEMY_COUNT * WAVE_GROWTH_RATE);
		state.wave.count_down = WAVE_DURATION;
		state.wave.number++;

		std::cerr << "wave " << state.wave.number << " spawning " << state.wave.baddies_to_spawn << std::endl;
	}

	if (state.wave.baddies_to_spawn > 0 && state.wave.spawn_cool_down <= 0)
	{
		auto next_baddie = state.next_generation[state.baddies.size() % state.next_generation.size()];

		auto spawn = state.wave.spawn_point;
		next_baddie.position = {(float)spawn[0] + us::randf(), 6.f, (float)spawn[1] + us::randf()};

		next_baddie.reset(state.wave.number);

		assert(next_baddie.position[0] >= 0);
		assert(next_baddie.position[2] >= 0);
		assert(next_baddie.position[0] < state.level->width());
		assert(next_baddie.position[2] < state.level->height());

		state.baddies.push_back(next_baddie);
		state.wave.baddies_to_spawn--;
		state.wave.spawn_cool_down = 10.f / (float)(state.wave.number * (WAVE_BASE_ENEMY_COUNT * WAVE_GROWTH_RATE));
	}

	state.wave.count_down -= dt;
	state.wave.spawn_cool_down -= dt;
}

} // namespace us