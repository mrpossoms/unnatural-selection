#pragma once

#define LYMPH_NODE_HP 100
#define PLAYER_SPEED 20

#define WEAPON_CARBINE_COOLDOWN 0.125
#define WEAPON_LASER_COOLDOWN 0.001
#define WEAPON_SHOTGUN_COOLDOWN 0.25

#define WEAPON_CARBINE_SPREAD 0.2
#define WEAPON_LASER_SPREAD 0.0
#define WEAPON_SHOTGUN_SPREAD 3.0

#define WEAPON_CARBINE_VEL 75
#define WEAPON_LASER_VEL 100
#define WEAPON_SHOTGUN_VEL 50

#define WEAPON_CARBINE_PROJECTILES 1
#define WEAPON_LASER_PROJECTILES 1
#define WEAPON_SHOTGUN_PROJECTILES 7

#define WAVE_BASE_ENEMY_COUNT 20
#define WAVE_GROWTH_RATE 0.1
#define WAVE_DURATION 30

enum trait
{
	armor = 0,
	shield = 1,
	unprotected = 2,
};

// rows are indexed by weapon type. Column indices correspond
// to attributes 0: armor, 1: shield, 2: unprotected
static mat<3, 3> damage_matrix = {
	{  2, 0.125, 1 }, // carbine
	{0.01,   2, 0.01 }, // laser
	{0.125, 0.125, 2 }, // shotgun
};