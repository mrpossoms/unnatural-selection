#pragma once

#include "state.hpp"

namespace us
{

static void update_projectiles(us::state& state, float dt)
{
    // update particles
	for (unsigned i = 0; i < state.projectiles.size(); i++)
	{
		us::projectile& projectile = state.projectiles[i];
		auto new_pos = projectile.position + projectile.velocity * dt;

		auto is_dead = !state.level->cells[(int)(new_pos[0] + 0.5)][(int)(new_pos[2] + 0.5)].is_floor ||
		               new_pos[1] <= 0 || new_pos[1] >= 6 ||
		               projectile.life <= 0;

		if (is_dead)
		{
			state.projectiles.remove_at(i);
			continue;
		}

		// deal damage to baddies
		for (unsigned j = 0; j < state.baddies.size(); j++)
		{
			auto& baddie = state.baddies[j];
			auto t = intersect::ray_sphere(projectile.position, projectile.velocity, baddie.position, 0.5f);

			if (t <= dt)
			{
				for (unsigned i = 6; i--;)
				state.particles.spawn(baddie.position, vec<3>{randf(), randf(), randf()} * 4, state.time, state.time + 10, {(rand() % 6)/6.f});

				projectile.life = 0;
				baddie.hp -= 1;
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

		auto& my_cell = state.level->cells[r][c];
		if (my_cell.lymph_node_hp > 0)
		{
			my_cell.lymph_node_hp -= baddie.genes.damage;
			baddie.hp = 0; // die on collision
		}

		if (baddie.hp <= 0)
		{
			state.baddies.remove_at(i);
			continue;
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
				baddie.velocity += (vec<3>{best_node->r + 0.5f, (randf() + 1) * 3, best_node->c + 0.5f} - baddie.position) * 0.1 * baddie.genes.speed;
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

 	vec<2> dir = {};
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_W) == GLFW_PRESS) dir += { 0, dt};
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_S) == GLFW_PRESS) dir += { 0,-dt};
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_A) == GLFW_PRESS) dir += {-dt, 0};
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_D) == GLFW_PRESS) dir += { dt, 0};
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_LEFT) == GLFW_PRESS) player.theta += (-dt);
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_RIGHT) == GLFW_PRESS) player.theta += (dt);
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_UP) == GLFW_PRESS) player.phi += (dt);
    if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_DOWN) == GLFW_PRESS) player.phi += (-dt);
    
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
    	// for (unsigned i = 0; i < wea)
    	// if (player.shoot(p))
    	// {
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

    state.player.walk(dir * player.speed);

	if (state.level->cells[(int)(new_pos[0] + 0.5)][(int)(new_pos[2] + 0.5)].is_floor)
	{
		player.position = new_pos;	
	}

	player.cool_down = std::max<float>(0, player.cool_down - dt);

	// position += velocity * dt;
	player.velocity *= 0.9f;

	player.phi = std::max<float>(-M_PI / 4, player.phi);
	player.phi = std::min<float>( M_PI / 4, player.phi);
}

} // namespace us