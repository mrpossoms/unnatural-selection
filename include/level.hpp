#pragma once
#include <algorithm>

#include <g.h>
#include "constants.hpp"

namespace us
{

using namespace g::gfx;
using namespace xmath;

struct level : public mesh<vertex::pos_uv_norm>
{

struct cell
{
	unsigned lymph_node_hp = 0;
	bool has_spawner = false;
	bool is_floor = false;
	std::unordered_map<unsigned, float> node_distances = {};
};

std::vector<std::vector<cell>> cells;
std::vector<vec<2, unsigned>> spawn_points;
std::vector<vec<2, unsigned>> lymph_nodes;

const vec<2, int> offsets[8] = {
	{-1, -1},
	{ 0, -1},
	{ 1, -1},

	{-1,  0},
	{ 1,  0},

	{-1, 1},
	{ 0, 1},
	{ 1, 1},
};

level() = default;

level(const texture& base)
{
	glGenBuffers(2, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	// build the level datastructure
	for (unsigned y = 0; y < base.size[1]; y++)
	{
		std::vector<cell> row;

		for (unsigned x = 0; x < base.size[0]; x++)
		{
			unsigned char* pixel = base.sample(x, y);
			uint8_t red   = pixel[0];
			uint8_t green = pixel[1];
			uint8_t blue  = pixel[2];
			cell grid_cell;

			if (red == 0xff) { grid_cell.is_floor = true; }
			if (green == 0xff) { grid_cell.lymph_node_hp = LYMPH_NODE_HP; }
			if (blue == 0xff) { grid_cell.has_spawner = true; }

			if (grid_cell.has_spawner)
			{
				spawn_points.push_back({y, x});
			}

			if (grid_cell.lymph_node_hp > 0)
			{
				lymph_nodes.push_back({y, x});
			}

			row.push_back(grid_cell);
		}

		cells.push_back(row);
	}

	// build nav-grid
	{
		unsigned i = 0;
		for (const auto& node : lymph_nodes)
		{
			// cells[node[0]][node[1]].node_distances[i] = 0;
			build_nav_grid(node[0], node[1], node[0], node[1], i);
			i++;
		}
	}
}

void for_each_neighbor(
	int r, int c, 
	std::function<void(cell& cell, unsigned r, unsigned c)> cb)
{
	for (unsigned i = 0; i < sizeof(offsets) / sizeof(offsets[0]); i++)
	{
		auto ri = r + offsets[i][0];
		auto ci = c + offsets[i][1];

		if (ri < 0 || ri >= height()) { continue; }
		if (ci < 0 || ci >= width()) { continue; }

		cb(cells[ri][ci], ri, ci);
	}
}

void build_nav_grid(int x, int y, int last_x, int last_y, unsigned node_id)
{
	if (x < 0 || y < 0 || x >= width() || y >= height()) { return; }
	if (!cells[x][y].is_floor) { return; }

	auto itr = cells[x][y].node_distances.find(node_id);
	if (cells[x][y].node_distances.end() != itr) { return; }

	// TODO: if id is added, escape

	auto dx = x - last_x;
	auto dy = y - last_y;

	float dist = cells[last_x][last_y].node_distances[node_id];
	for_each_neighbor(x, y, [&](cell& cell, unsigned r, unsigned c) {
		auto itr = cell.node_distances.find(node_id);
		if (cell.node_distances.end() != itr)
		{
			dist = std::min<float>(cell.node_distances[node_id], dist);
		}
	});

	cells[x][y].node_distances[node_id] = dist + sqrtf(dx * dx + dy * dy);

	for (unsigned i = 0; i < sizeof(offsets) / sizeof(vec<2, int>); i++)
	{
		build_nav_grid(x + offsets[i][0], y + offsets[i][1], x, y, node_id);		
	}
}

inline unsigned width() const { return cells[0].size(); }
inline unsigned height() const { return cells.size(); }
inline unsigned floor_area() const
{
	unsigned area = 0;
	for (auto& row : cells)
	{
		for (auto& col : row)
		{
			area += col.is_floor;
		}
	}

	return area;
}

std::string info() const
{
	std::string str = "(" + std::to_string(width()) + ", " + std::to_string(height()) + ")\n";
	str += "floor area: " + std::to_string(floor_area()) + "\n";
	str += "lymph_nodes: " + std::to_string(lymph_nodes.size()) + "\n";
	return str;
}

};

} // namespace us