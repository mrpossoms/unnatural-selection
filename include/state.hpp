#pragma once
#include "level.hpp"
#include "particles.hpp"

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
		uint8_t target_node;
		uint8_t hp;
		uint8_t speed;
		uint8_t armor;
		uint8_t shield;
		uint8_t damage;

		float sum() { return hp + speed + armor + shield + damage + 1; }
	};

	genome genes;
	float hp = 1;
	float speed = 0;
	float armor = 0;
	float shield = 0;
	float damage = 0;

	float damage_dealt = 0;
	float damage_taken = 0;
	float progress = 0;
	bool idle = false;

	uint8_t* genome_buf()
	{
		return (uint8_t *)&genes;
	}

	size_t genome_size()
	{
		return sizeof(genes); // TODO: expand to more traits
	}

	float score() const
	{
		return damage_dealt * 100 + progress; // TODO
	}

	void initialize()
	{
		genes.target_node = rand() % 256;
		genes.hp = rand() % 256;
		genes.speed = rand() % 256;
		genes.armor = rand() % 256;
		genes.shield = rand() % 256;
		genes.damage = rand() % 256;
	}

	void reset(unsigned wave)
	{
		float points = wave;
		float sum = genes.sum();
		hp = points * (genes.hp / sum) + 0.1f;
		speed = 1 + points * (genes.speed / sum);
		armor = points * (genes.armor / sum);
		shield = points * (genes.shield / sum);
		damage = points * (genes.damage / sum);

		damage_dealt = 0;
		damage_taken = 0;
		progress = 0;

		velocity = {};
	}

	void take_hit(const projectile& p)
	{
		float base_damage = 1;

		auto shield_damage = std::min<float>(shield, base_damage * damage_matrix[(size_t)p.type][trait::shield]);
		shield -= shield_damage;
		base_damage -= shield_damage / damage_matrix[(size_t)p.type][trait::shield];

		auto armor_damage = std::min<float>(armor, base_damage * damage_matrix[(size_t)p.type][trait::armor]);
		armor -= armor_damage;

		auto hp_damage = std::min<float>(hp, base_damage * damage_matrix[(size_t)p.type][trait::unprotected]);
		hp -= hp_damage;

		damage_taken += (shield_damage + armor_damage + hp_damage);

		// if (shield > 0)
		// {
		// 	shield -= damage_matrix[(size_t)p.type][trait::shield];
		// 	roll_over = (shield < 0) * shield;

		// }
		// if (armor > 0)
		// {
		// 	armor += roll_over;
		// 	armor -= damage_matrix[(size_t)p.type][trait::armor];
		// 	roll_over = (armor < 0) * armor;
		// }
		// else if (hp > 0)
		// {
		// 	hp += roll_over;
		// 	hp -= damage_matrix[(size_t)p.type][trait::unprotected];
		// }

		// std::cerr << "sheild: " << shield << " armor: " << armor << " hp: " << hp << std::endl;
	}
};


struct player : public g::game::camera_perspective
{
	float speed = PLAYER_SPEED;

	float theta = 0;
	float phi = 0;
	float cool_down = 0;
	unsigned selected_weapon = 0;
	bool is_sprinting = false;
	float gun_shoot;

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

	float randf() { return ((rand() % 2048) / 1024.f) - 1.f; }

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
	struct {
		unsigned number = 0;
		float count_down = 5;
		unsigned baddies_to_spawn = 0;
		vec<2, unsigned> spawn_point;
		float spawn_cool_down = 0;
	} wave;

	float time = 0;
	std::shared_ptr<us::level> level;
	us::player player;

	unsigned onboarding_step = 0;
	bool onboarding_done = false;

	g::bounded_list<us::projectile, 100> projectiles;
	std::vector<us::baddie> baddies;
	std::vector<us::baddie> next_generation;

	std::vector<vec<12>> particle_spawn_queue;

	particle_system<128> gibs;
	particle_system<128> smoke;
	particle_system<128> chunks;
	g::snd::source ambient;
	g::snd::source wave_start;
	g::snd::source_ring virus_sounds;
	g::snd::source_ring impacts[8];
	g::snd::source_ring wall_impacts[2];
	g::snd::source_ring node_damage[2];
	g::snd::source_ring gun_sounds[3];
};

} // namespace us
