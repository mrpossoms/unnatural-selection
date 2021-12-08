#pragma once
#include "level.hpp"

using namespace xmath;

namespace us
{

struct projectile : public g::dyn::particle
{
	enum class type
	{
		bullet = 0,
		laser,
		rocket
	};

	projectile::type type;
	float life = 10;

	projectile() = default;
};


// struct weapon
// {
// 	float cool_down
// }


struct player : public g::game::camera_perspective
{
	float theta = 0;
	float phi = 0;

	unsigned selected_weapon = 0;

	vec<3> velocity;

	xmath::quat<> get_orientation()
	{
		return quat<>::from_axis_angle({0, 1, 0}, theta) * quat<>::from_axis_angle({1, 0, 0}, phi);
	}

	vec<3> walk_forward()
	{
		return quat<>::from_axis_angle({0, 1, 0}, theta).rotate({0, 0, -1});
	}

	void walk(const vec<2>& u)
	{
		auto dir = walk_forward() * u[1] + left() * u[0];

		velocity += dir;
	}

	void update(float dt, const us::level& level)
	{
		auto new_pos = position + velocity * dt;

		if (level.cells[(int)(new_pos[0] + 0.5)][(int)(new_pos[2] + 0.5)].is_floor)
		{
			position = new_pos;	
		}

		// position += velocity * dt;
		velocity *= 0.9f;//(1.f - (1.f/(dt+0.00001f)));

		phi = std::max<float>(-M_PI / 4, phi);
		phi = std::min<float>( M_PI / 4, phi);
	}
};

struct state
{
	float time;
	std::shared_ptr<us::level> level;
	us::player player;

	g::bounded_list<us::projectile, 100> projectiles;
};

} // namespace us