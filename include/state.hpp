#pragma once
#include "level.hpp"

namespace us
{

struct player : public g::game::camera_perspective
{

};

struct state
{
	std::shared_ptr<us::level> level;
	us::player player;
};

} // namespace us