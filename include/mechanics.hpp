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
			i--;
		}

		projectile.position = new_pos;
		projectile.life -= dt;
	}
}

} // namespace us