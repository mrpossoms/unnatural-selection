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
		// auto is_dead = !state.level->cells[(int)(new_pos[0] + 0.5)][(int)(new_pos[2] + 0.5)].is_floor ||
		//                new_pos[1] <= 0 || new_pos[1] >= 6 ||
		//                projectile.life <= 0;

		if (baddie.hp <= 0)
		{
			state.baddies.remove_at(i);
			continue;
		}

		{ // do path finding
			int r = baddie.position[0]; int c = baddie.position[2];
			level::cell* best_node = nullptr;
			float best_dist = 10e6;

			// look at neighboring nav nodes for the cell with the smallest distance to the target
			state.level->for_each_neighbor(r, c, [&](level::cell& neighbor, unsigned nr, unsigned nc) -> bool {
				auto itr = neighbor.node_distances.find(0);
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
				baddie.velocity += (vec<3>{best_node->r + 0.5f, (randf() + 1) * 3, best_node->c + 0.5f} - baddie.position) * 0.5;
			}
		}

		auto new_pos = baddie.position + baddie.velocity * dt;
		baddie.position = new_pos;
		baddie.velocity *= 0.9f;
		// projectile.life -= dt;
	}	
}

} // namespace us