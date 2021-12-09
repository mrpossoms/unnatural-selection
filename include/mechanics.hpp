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
    // update particles
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

		{ // do path finding

			level::cell* best_node = nullptr;
			float best_dist = 10e6;

			// look at neighboring nav nodes for the cell with the smallest distance to the target
			state.level->for_each_neighbor(r, c, [&](level::cell& neighbor, unsigned nr, unsigned nc) -> bool {
				auto itr = neighbor.node_distances.find(baddie.genes.target_node % state.level->living_lymph_nodes());
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

} // namespace us