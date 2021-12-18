#pragma once
#include <algorithm>

#include <g.h>
#include "constants.hpp"

namespace us
{

using namespace g::gfx;
using namespace xmath;

struct level
{

struct cell
{
	unsigned r = 0, c = 0;
	unsigned lymph_node_hp = 0;
	bool has_spawner = false;
	bool is_floor = false;
	bool wall_start = false;
	int wall_id = -1;
	cell* wall_next = nullptr;
	std::unordered_map<unsigned, float> node_distances = {};
};

std::vector<std::vector<cell>> cells;
std::vector<vec<2, unsigned>> spawn_points;
std::vector<vec<2, unsigned>> lymph_nodes;
std::unordered_map<unsigned, cell*> walls;
unsigned hash = 0;

// const vec<2, int> offsets[8] = {
// 	{-1, -1},
// 	{ 0, -1},
// 	{ 1, -1},
// 	{ 1,  0},
// 	{ 1,  1},
// 	{ 0,  1},
// 	{ 1,  1},
// 	{-1,  0},
// };

level() = default;

level(const texture& base)
{
	// build the level datastructure
	for (unsigned r = 0; r < base.size[1]; r++)
	{
		std::vector<cell> row;

		for (unsigned c = 0; c < base.size[0]; c++)
		{
			unsigned char* pixel = base.sample(c, r);
			uint8_t red   = pixel[0];
			uint8_t green = pixel[1];
			uint8_t blue  = pixel[2];
			cell grid_cell;
			grid_cell.r = r;
			grid_cell.c = c;

			if (red == 0xff) { grid_cell.is_floor = true; }
			if (green == 0xff) { grid_cell.lymph_node_hp = LYMPH_NODE_HP; }
			if (blue == 0xff) { grid_cell.has_spawner = true; }

			hash += (red << 16 | green << 8 | blue);

			if (grid_cell.has_spawner)
			{
				spawn_points.push_back({r, c});
			}

			if (grid_cell.lymph_node_hp > 0)
			{
				lymph_nodes.push_back({r, c});
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

	// assign wall ids
	unsigned wall_count = 0;
	for (unsigned r = 0; r < base.size[1]; r++)
	{
		for (unsigned c = 0; c < base.size[0]; c++)
		{
			if (is_boundary(r, c) >= 2 && cells[r][c].wall_id == -1)
			{
				walls[wall_count] = &cells[r][c];
				cells[r][c].wall_start = true;
				build_wall(r, c, wall_count);
				wall_count++;
			}
		}
	}

	for (unsigned r = 0; r < base.size[1]; r++)
	{
		std::string row = "";
		for (unsigned c = 0; c < base.size[0]; c++)
		{
			// if (cells[r][c].node_distances.size() > 0)
			// {
			// 	row += "^";
			// 	continue;
			// }

			if (cells[r][c].is_floor)
			{
				if (cells[r][c].lymph_node_hp > 0)
				{
					row += "N";
				}
				else if (cells[r][c].has_spawner)
				{
					row += "S";
				}
				else
				{
					row += " ";
				}
			}
			else
			{
				if (cells[r][c].wall_next)
				{
					row += std::to_string(cells[r][c].wall_id);
				}
				else if (r == 0 && c == 0)
				{
					row += "0";
				}
				else if (c == width()-1)
				{
					row += "r";
				}
				else
				{
					row += ".";
				}
			}
		}

		std::cerr << row << std::endl;
	}
}

std::vector<unsigned> living_lymph_nodes()
{
	std::vector<unsigned> living;

	for (unsigned i = 0; i < lymph_nodes.size(); i++)
	{
		if (cells[lymph_nodes[i][0]][lymph_nodes[i][1]].lymph_node_hp > 0) { living.push_back(i); }
	}

	return living;
}

void build_wall(unsigned r, unsigned c, int id, int depth=0)
{
	cells[r][c].wall_id = id;

	unsigned best_bound = 0;
	cell* best = nullptr;

	for_each_neighbor(r, c, [&](cell& neighbor, unsigned nr, unsigned nc) -> bool {
		auto bound = is_boundary(nr, nc);

		if ((best_bound < bound && neighbor.wall_id == -1) || (neighbor.wall_start && depth > 2))
		{
			best_bound = bound;
			best = &neighbor;
			// br = nr;
			// bc = nc;
			// return true;
		}

		return false;
	});

	if (best)
	{
		cells[r][c].wall_next = best;

		if (best->wall_id == -1)
		{
			build_wall(best->r, best->c, id, depth + 1);
		}
		else
		{
			// cells[r][c].wall_next = &neighbor;
			if (best->wall_start) { std::cerr << best->r << "," << best->c << " reached start" << std::endl; }
		}		
	}
}

unsigned is_boundary(unsigned r, unsigned c)
{
	if (cells[r][c].is_floor) { return false; }

	int count = 0;
	for_each_neighbor(r, c, [&](cell& neighbor, unsigned r, unsigned c) -> bool {
		// if (neighbor.is_floor) { boundary = true; }
		count += neighbor.is_floor;
		return false;
	});

	return count;
}

void for_each_neighbor(
	int r, int c, 
	std::function<bool(cell& cell, unsigned r, unsigned c)> cb)
{
	int dr = 0, dc = 0;

	for (dr = -1; dr <= 1; dr++)
	for (dc = -1; dc <= 1; dc++)
	{
		if (dr == 0 && dc == 0) { continue; }

		auto ri = r + dr;
		auto ci = c + dc;
		if (ri < 0 || ri >= (int)height()) { continue; }
		if (ci < 0 || ci >= (int)width()) { continue; }

		if (cb(cells[ri][ci], ri, ci)) { break; }
	}
}

void build_nav_grid(int x, int y, int last_x, int last_y, unsigned node_id)
{
	if (x < 0 || y < 0 || x >= (int)width() || y >= (int)height()) { return; }
	if (!cells[x][y].is_floor) { return; }

	auto itr = cells[x][y].node_distances.find(node_id);
	if (cells[x][y].node_distances.end() != itr) { return; }

	// TODO: if id is added, escape

	auto dx = x - last_x;
	auto dy = y - last_y;

	float dist = cells[last_x][last_y].node_distances[node_id];
	for_each_neighbor(x, y, [&](cell& cell, unsigned r, unsigned c) -> bool {
		auto itr = cell.node_distances.find(node_id);
		if (cell.node_distances.end() != itr)
		{
			dist = std::min<float>(cell.node_distances[node_id], dist);
		}

		return false;
	});

	cells[x][y].node_distances[node_id] = dist + sqrtf(dx * dx + dy * dy);

	for_each_neighbor(x, y, [&](cell& cell, unsigned r, unsigned c) -> bool {
		build_nav_grid(r, c, x, y, node_id);
		return false;
	});

	// for (unsigned i = 0; i < sizeof(offsets) / sizeof(vec<2, int>); i++)
	// {
	// 	build_nav_grid(x + offsets[i][0], y + offsets[i][1], x, y, node_id);		
	// }
}

cell& get_cell(const vec<3>& p)
{
	int r = std::max<int>(std::min<int>(height()-1, (int)p[0]), 0);
	int c = std::max<int>(std::min<int>(width()-1, (int)p[2]), 0);

	return cells[r][c];
}

cell& operator[](const vec<3>& p)
{
	int r = std::max<int>(std::min<int>(height(), (int)p[2]), 0);
	int c = std::max<int>(std::min<int>(width(), (int)p[0]), 0);

	return cells[r][c];
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