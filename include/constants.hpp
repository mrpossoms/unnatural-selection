#pragma once

#define LYMPH_NODE_HP 100
#define PLAYER_SPEED 20

#define WEAPON_CARBINE_COOLDOWN 0.25
#define WEAPON_LASER_COOLDOWN 0.01
#define WEAPON_SHOTGUN_COOLDOWN 0.25

#define WEAPON_CARBINE_SPREAD 0.2
#define WEAPON_LASER_SPREAD 0.0
#define WEAPON_SHOTGUN_SPREAD 1.25

#define WEAPON_CARBINE_VEL 50
#define WEAPON_LASER_VEL 10
#define WEAPON_SHOTGUN_VEL 20

#define WEAPON_CARBINE_PROJECTILES 1
#define WEAPON_LASER_PROJECTILES 1
#define WEAPON_SHOTGUN_PROJECTILES 7

// rows are indexed by weapon type. Column indices correspond
// to attributes 0: armor, 1: shield, 2: unprotected
static mat<3, 3> damage_matrix = {
	{  2,   0, 1 }, // carbine
	{  0,   2, 1 }, // laser
	{0.5, 0.5, 2 }, // shotgun
};