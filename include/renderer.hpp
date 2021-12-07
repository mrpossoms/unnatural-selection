#pragma once
#include "g.h"
#include "state.hpp"


using mat4 = xmath::mat<4,4>;

namespace us
{

struct renderer
{
	unsigned level_hash = 0;
	g::gfx::mesh<g::gfx::vertex::pos_uv_norm> level_mesh;

	void build_level_mesh(const std::shared_ptr<us::level> level)
	{
		level_mesh = mesh_factory::empty_mesh<g::gfx::vertex::pos_uv_norm>();

		const auto ceiling = 6;

		std::vector<g::gfx::vertex::pos_uv_norm> vertices;
		std::vector<uint32_t> indices;

		for (auto& wall_kvp : level->walls)
		{
			level::cell* start = wall_kvp.second;
			level::cell* curr = start;

			vec<2> min_corner = {}, max_corner = {};

			while (curr->wall_next)
			{
				auto next = curr->wall_next;
				uint32_t n = vertices.size();

				vertices.push_back({{ (float)curr->r, 0, (float)curr->c }, {1, 1}, {1, 0, 0}});
		        vertices.push_back({{ (float)next->r, 0, (float)next->c }, {0, 1}, {1, 0, 0}});
		        vertices.push_back({{ (float)next->r, ceiling, (float)next->c }, {0, 0}, {1, 0, 0}});
		        vertices.push_back({{ (float)curr->r, ceiling, (float)curr->c }, {1, 0}, {1, 0, 0}});

		        min_corner.take_min({(float)curr->r, (float)curr->c});
		        max_corner.take_max({(float)curr->r, (float)curr->c});

		        indices.push_back(n + 2);
		        indices.push_back(n + 3);
		        indices.push_back(n + 0);
		        indices.push_back(n + 1);
		        indices.push_back(n + 2);
		        indices.push_back(n + 0);

		        curr = next;

		        if (curr == start) { break; }
			}

			float w = level->width();
			float h = level->height();

			{ // floor
				uint32_t n = vertices.size();
				vertices.push_back({{ 0, 0, 0 }, {w, h}, {0, 1, 0}});
				vertices.push_back({{ w, 0, 0 }, {0, h}, {0, 1, 0}});
				vertices.push_back({{ w, 0, h }, {0, 0}, {0, 1, 0}});
				vertices.push_back({{ 0, 0, h }, {w, 0}, {0, 1, 0}});

				indices.push_back(n + 2);
				indices.push_back(n + 3);
				indices.push_back(n + 0);
				indices.push_back(n + 1);
				indices.push_back(n + 2);
				indices.push_back(n + 0);
			}

			{ // ceiling
				uint32_t n = vertices.size();
				vertices.push_back({{ 0, ceiling, 0 }, {w, h}, {0, 1, 0}});
				vertices.push_back({{ w, ceiling, 0 }, {0, h}, {0, 1, 0}});
				vertices.push_back({{ w, ceiling, h }, {0, 0}, {0, 1, 0}});
				vertices.push_back({{ 0, ceiling, h }, {w, 0}, {0, 1, 0}});

				indices.push_back(n + 2);
				indices.push_back(n + 3);
				indices.push_back(n + 0);
				indices.push_back(n + 1);
				indices.push_back(n + 2);
				indices.push_back(n + 0);
			}
		}

		g::gfx::mesh<g::gfx::vertex::pos_uv_norm>::compute_normals(vertices, indices);

		level_mesh.set_vertices(vertices);
		level_mesh.set_indices(indices);


		level_hash = level->hash;
	}

	void draw(g::asset::store& assets, us::state& state)
	{
		if (state.level->hash != level_hash)
		{
			build_level_mesh(state.level);
		}

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        level_mesh.using_shader(assets.shader("level.vs+level.fs"))
            ["u_model"].mat4(mat4::I())
            .set_camera(state.player)
            // .draw<GL_POINTS>();
            .draw<GL_TRIANGLES>();
	}
};

}