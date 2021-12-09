#pragma once
#include "g.h"
#include "state.hpp"
#include "nlohmann/json.hpp"

using mat4 = xmath::mat<4,4>;

namespace us
{

struct sprite
{
	vec<2> frame_dims = {};
	unsigned frames = 0;
	float duration = 0;

	sprite() = default;

	sprite(const nlohmann::json& json)
	{
		for (auto& frame : json["frames"])
		{
			float w = frame["sourceSize"]["w"];
			float sheet_w = json["meta"]["size"]["w"];
			duration = frame["duration"];
			duration *= 0.1f;
			frame_dims = vec<2>{ w / sheet_w, 1 };
			frames += 1;
		}
	}

};

struct renderer
{
	unsigned level_hash = 0;
	g::gfx::mesh<g::gfx::vertex::pos_uv_norm> level_mesh;
	g::gfx::mesh<g::gfx::vertex::pos_uv_norm> billboard_mesh;
	std::unordered_map<std::string, sprite> sprites;
	std::unordered_map<std::string, nlohmann::json> json;

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

				auto forward = vec<3>{ (float)(next->r - curr->r), 0, (float)(next->c - curr->c) };
				auto normal = vec<3>::cross(forward, {0, 1, 0}).unit();

				vertices.push_back({{ (float)curr->r, 0, (float)curr->c }, {1, 1}, normal});
		        vertices.push_back({{ (float)next->r, 0, (float)next->c }, {0, 1}, normal});
		        vertices.push_back({{ (float)next->r, ceiling, (float)next->c }, {0, 0}, normal});
		        vertices.push_back({{ (float)curr->r, ceiling, (float)curr->c }, {1, 0}, normal});

		        min_corner.take_min({(float)curr->r, (float)curr->c});
		        max_corner.take_max({(float)curr->r, (float)curr->c});

				indices.push_back(n + 0);
				indices.push_back(n + 3);
				indices.push_back(n + 2);
				indices.push_back(n + 0);
				indices.push_back(n + 2);
				indices.push_back(n + 1);

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
				vertices.push_back({{ 0, ceiling, 0 }, {w, h}, {0, -1, 0}});
				vertices.push_back({{ w, ceiling, 0 }, {0, h}, {0, -1, 0}});
				vertices.push_back({{ w, ceiling, h }, {0, 0}, {0, -1, 0}});
				vertices.push_back({{ 0, ceiling, h }, {w, 0}, {0, -1, 0}});

				indices.push_back(n + 0);
				indices.push_back(n + 3);
				indices.push_back(n + 2);
				indices.push_back(n + 0);
				indices.push_back(n + 2);
				indices.push_back(n + 1);
			}
		}

		// g::gfx::mesh<g::gfx::vertex::pos_uv_norm>::compute_normals(vertices, indices);

		level_mesh.set_vertices(vertices);
		level_mesh.set_indices(indices);


		level_hash = level->hash;
	}

	sprite& get_sprite(const std::string& name)
	{

		auto itr = sprites.find(name + ".json");
		if (itr == sprites.end())
		{
			std::ifstream ifs("data/tex/" + name + ".json");

			us::sprite sprite(nlohmann::json::parse(ifs));
			sprites.insert({name + ".json", sprite});
		}

		return sprites[name + ".json"];
	}

	void draw(g::asset::store& assets, us::state& state)
	{
		if (state.level->hash != level_hash)
		{
			build_level_mesh(state.level);
		}

		if (!billboard_mesh.is_initialized())
		{
			billboard_mesh = g::gfx::mesh_factory::plane();
		}
 
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		state.player.aspect_ratio(g::gfx::aspect());

		level_mesh.using_shader(assets.shader("level.vs+level.fs"))
		    ["u_model"].mat4(mat4::I())
		    ["u_floor"].texture(assets.tex("floor_0.repeating.png"))
		    ["u_wall"].texture(assets.tex("Wall_0.repeating.png"))
		    .set_camera(state.player)
		    // .draw<GL_POINTS>();
		    .draw<GL_TRIANGLES>();

		// render pustules
		auto& node_mesh = assets.geo("node.obj");
		for (auto& lymph_node : state.level->lymph_nodes)
		{
			if (state.level->cells[lymph_node[0]][lymph_node[1]].lymph_node_hp > 0)
			{
				node_mesh.using_shader(assets.shader("level.vs+model.fs"))
				    ["u_model"].mat4(mat4::translation(vec<3>{(float)lymph_node[0], 0, (float)lymph_node[1]}))
				    ["u_texture"].texture(assets.tex("nodeTexture.png"))
				    .set_camera(state.player)
				    // .draw<GL_POINTS>();
				    .draw<GL_TRIANGLES>();
			}
		}

		// glDisable(GL_DEPTH_TEST);

		// projectiles
		for (unsigned i = 0; i < state.projectiles.size(); i++)
		{
			us::projectile& projectile = state.projectiles[i];
			const std::string imgs[] = {
				"bullet_0.png",
				"bullet_1.png",
				"bullet_2.png",
				"bullet_3.png",
			};

			billboard_mesh.using_shader(assets.shader("billboard.vs+animated_sprite.fs"))
				["u_position"].vec3(projectile.position)
				["u_sprite_sheet"].texture(assets.tex(imgs[i%4]))
				["u_frame_dims"].vec2({1, 1})
				["u_frame"].int1(0)
				.set_camera(state.player)
				.draw<GL_TRIANGLE_FAN>();
		}

		// draw the baddies
		// glDisable(GL_DEPTH_TEST);
		for (unsigned i = 0; i < state.baddies.size(); i++)
		{
			auto& baddie = state.baddies[i];
			const std::string imgs[] = {
				"virus_1.png",
				"virus_2.png",
				"virus_3.png",
			};

			const std::string jsons[] = {
				"virus_1",
				"virus_2",
				"virus_3",
			};

			auto& meta = get_sprite(jsons[i % 3]);

			billboard_mesh.using_shader(assets.shader("billboard.vs+animated_sprite.fs"))
				["u_position"].vec3(baddie.position)
				["u_sprite_sheet"].texture(assets.tex(imgs[i%3]))
				["u_frame_dims"].vec2(meta.frame_dims)
				["u_frame"].int1(static_cast<int>((state.time * meta.duration) + (i * 10)) % meta.frames)
				.set_camera(state.player)
				.draw<GL_TRIANGLE_FAN>();
		}
		// glEnable(GL_DEPTH_TEST);


		// glDisable(GL_DEPTH_TEST);
		// for (unsigned r = 0; r < state.level->height(); r++)
		// for (unsigned c = 0; c < state.level->width(); c++)
		// {
		// 	if (state.level->cells[r][c].node_distances.size() == 0) { continue; }
		// 	g::gfx::debug::print{&state.player}.color({0, 1, 0, 1}).ray(vec<3>{(float)r, 0, (float)c}, {0, 1, 0});// state.level->cells[r][c]
		// }
		// glEnable(GL_DEPTH_TEST);


			
	}
};

}