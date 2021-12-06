#include <g.h>

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
	std::unordered_map<unsigned, float> node_distances;
};

level() = default;

level(const texture& base)
{
	glGenBuffers(2, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	// build the level datastructure
	for (unsigned r = 0; r < base.size[0]; r++)
	{
		cells.push_back({});

		for (unsigned c = 0; c < base.size[1]; c++)
		{
			unsigned char* pixel = base.sample(c, r);
			uint8_t r = pixel[0];
			uint8_t g = pixel[1];
			uint8_t b = pixel[2];
			cell grid_cell;

			if (r == 0xff) { grid_cell.is_floor = true; }
			if (g == 0xff) { grid_cell.lymph_node_hp = 100; /* TODO */ }
			if (b == 0xff) { grid_cell.has_spawner = true; }

			if (grid_cell.has_spawner)
			{
				spawn_points.push_back({r, c});
			}

			cells[r].push_back(grid_cell);
		}
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

void build_nav_grid(int x, int y, int last_x, int last_y, unsigned node_id)
{
	if (x < 0 || y < 0 || x >= width() || y >= height()) { return; }

	auto itr = cells[y][x].node_distances.find(node_id);
	if (cells[y][x].node_distances.end() != itr) { return; }

	// TODO: if id is added, escape

	auto dx = x - last_x;
	auto dy = y - last_y;

	cells[y][x].node_distances[node_id] = cells[last_y][last_x].node_distances[node_id] + sqrtf(dx * dx + dy * dy);

	build_nav_grid(x - 1, y - 1, x, y, node_id);
	build_nav_grid(x - 0, y - 1, x, y, node_id);
	build_nav_grid(x + 1, y - 1, x, y, node_id);

	build_nav_grid(x - 1, y + 0, x, y, node_id);
	build_nav_grid(x + 1, y + 0, x, y, node_id);
	
	build_nav_grid(x - 1, y + 1, x, y, node_id);
	build_nav_grid(x - 0, y + 1, x, y, node_id);
	build_nav_grid(x + 1, y + 1, x, y, node_id);
}

inline unsigned width() const { return cells[0].size(); }
inline unsigned height() const { return cells.size(); }

private:
	std::vector<std::vector<cell>> cells;
	std::vector<vec<2, unsigned>> spawn_points;
	std::vector<vec<2, unsigned>> lymph_nodes;

};

} // namespace us