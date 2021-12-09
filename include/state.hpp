#pragma once
#include "level.hpp"

using namespace xmath;

namespace us
{

float randf() { return ((rand() % 2048) / 1024.f) - 1.f; }

struct projectile : public g::dyn::particle
{
	enum class type
	{
		bullet = 0,
		laser,
		rocket
	};

	projectile::type type = projectile::type::bullet;
	float life = 10;

	projectile() = default;
};


// struct weapon
// {
// 	float cool_down
// }



struct baddie : public g::dyn::particle
{
	struct genome
	{
		uint8_t hp;
		uint8_t speed;
		uint8_t armor;
		uint8_t shield;
		uint8_t damage;
		uint8_t target_node;
	};

	genome genes;
	int hp = 1;
	float score = 0;
};


struct player : public g::game::camera_perspective
{
	float theta = 0;
	float phi = 0;
	float cool_down = 0;

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

	float randf() { return ((random() % 2048) / 1024.f) - 1.f; }

	bool shoot(projectile& p)
	{
		if (cool_down <= 0)
		{
			const auto spread = 0.5;
			p.position = position + forward() * 0.25f;
			p.velocity = forward() * 30 + (up() * randf() * spread) + (left() * randf() * spread);
			p.life = 10;
			cool_down = 0.1;

			return true;
		}

		return false;
	}

	void update(float dt, const us::level& level)
	{
		auto new_pos = position + velocity * dt;

		if (level.cells[(int)(new_pos[0] + 0.5)][(int)(new_pos[2] + 0.5)].is_floor)
		{
			position = new_pos;	
		

		}



		cool_down = std::max<float>(0, cool_down - dt);

		// position += velocity * dt;
		velocity *= 0.9f;

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
	g::bounded_list<us::baddie, 10000> baddies;
};

} // namespace us