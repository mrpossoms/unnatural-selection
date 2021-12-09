#pragma once
#include "level.hpp"

using namespace xmath;

namespace us
{

float randf() { return ((rand() % 2048) / 1024.f) - 1.f; }

struct projectile : public g::dyn::particle
{
	enum category
	{
		bullet = 0,
		laser = 1,
		pellet = 2
	};


	projectile::category type = projectile::category::bullet;
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
	float hp = 1;
	float armor = 0;
	float shield = 0;
	float score = 0;

	void take_hit(const projectile& p)
	{
		auto base_damage = damage_matrix[(size_t)p.type][2];
		// auto base_damage = damage_matrix[(size_t)p.type][2];
	}
};


struct player : public g::game::camera_perspective
{
	float speed = PLAYER_SPEED;

	float theta = 0;
	float phi = 0;
	float cool_down = 0;
	unsigned selected_weapon = 0;

	float weapon_velocities[3] = { WEAPON_CARBINE_VEL, WEAPON_LASER_VEL, WEAPON_SHOTGUN_VEL };
	float weapon_cool_downs[3] = { WEAPON_CARBINE_COOLDOWN, WEAPON_LASER_COOLDOWN, WEAPON_SHOTGUN_COOLDOWN };
	float weapon_spreads[3] = { WEAPON_CARBINE_SPREAD, WEAPON_LASER_SPREAD, WEAPON_SHOTGUN_SPREAD };
	unsigned weapon_projectiles[3] = { WEAPON_CARBINE_PROJECTILES, WEAPON_LASER_PROJECTILES, WEAPON_SHOTGUN_PROJECTILES };


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

	// bool shoot(projectile& p)
	// {
	// 	if (cool_down <= 0)
	// 	{
	// 		const auto spread = weapon_spreads[selected_weapon];
	// 		p.type = (us::projectile::type)selected_weapon;
	// 		p.position = position + forward() * 0.25f;
	// 		p.velocity = forward() * weapon_velocities[selected_weapon] + (up() * randf() * spread) + (left() * randf() * spread);
	// 		p.life = 10;
	// 		cool_down = weapon_cool_downs[selected_weapon];

	// 		return true;
	// 	}

	// 	return false;
	// }
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