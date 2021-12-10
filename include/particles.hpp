#pragma once
#include <g.h>
#include "nlohmann/json.hpp"

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

template <size_t CAP>
struct particle_system
{
	struct vertex
	{
		vec<3> position;
		vec<2> uv;
		uint32_t index;

		static void attributes(GLuint prog)
		{
			auto pos_loc = glGetAttribLocation(prog, "a_position");
			auto uv_loc = glGetAttribLocation(prog, "a_uv");
			auto idx_loc = glGetAttribLocation(prog, "a_index");

			if (pos_loc > -1) glEnableVertexAttribArray(pos_loc);
			if (uv_loc > -1) glEnableVertexAttribArray(uv_loc);
			if (idx_loc > -1) glEnableVertexAttribArray(idx_loc);

			auto p_size = sizeof(position);
			auto uv_size = sizeof(uv);

			if (pos_loc > -1) glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, sizeof(particle_system::vertex), (void*)0);
			if (uv_loc > -1) glVertexAttribPointer(uv_loc, 2, GL_FLOAT, false, sizeof(particle_system::vertex), (void*)p_size);
			if (idx_loc > -1) glVertexAttribPointer(idx_loc, 1, GL_UNSIGNED_INT, false, sizeof(particle_system::vertex), (void*)(p_size + uv_size));
		}
	};

	sprite sprite_meta;
	g::gfx::mesh<particle_system::vertex> particles_mesh;
	vec<3> positions[CAP];
	vec<3> velocities[CAP];
	vec<2> birth_deaths[CAP];
	vec<2> uv_offsets[CAP];
	int next = 0;

	particle_system()
	{
		for (unsigned i = 0; i < CAP; i++)
		{
			positions[i] = { 0, (float)i, 0 };
		}
	}

	void initialize()
	{
		std::vector<particle_system::vertex> vertices;
		std::vector<uint32_t> indices;

		for (uint16_t i = 0; i < 1; i++)
		{
			auto n = vertices.size();
			vertices.push_back({{ 0, 0, 0 }, {1, 1}, 0});
			vertices.push_back({{ 1, 0, 0 }, {0, 1}, 0});
			vertices.push_back({{ 1, 1, 0 }, {0, 0}, 0});
			vertices.push_back({{ 0, 1, 0 }, {1, 0}, 0});

			indices.push_back(n + 0);
			indices.push_back(n + 3);
			indices.push_back(n + 2);
			indices.push_back(n + 0);
			indices.push_back(n + 2);
			indices.push_back(n + 1);
		}

		particles_mesh = g::gfx::mesh_factory::empty_mesh<particle_system::vertex>();

		particles_mesh.set_vertices(vertices);
		particles_mesh.set_indices(indices);

	}


	void spawn(const vec<3>& position, const vec<3>& velocity, float birth_time, float death_time, const vec<2>& uv_offset)
	{
		positions[next] = position;
		velocities[next] = velocity;
		birth_deaths[next] = { birth_time, death_time };
		uv_offsets[next] = uv_offset;
		next = (next + 1) % CAP;
	}


	void draw(g::gfx::shader& shader, g::gfx::texture& sprite_sheet, g::game::camera& cam, float time)
	{
		for (unsigned i = 0; i < CAP; i++)
		{
			particles_mesh.using_shader(shader)
			    ["u_sprite_sheet"].texture(sprite_sheet)
			    ["u_positions"].vec3(positions[i])//, CAP)
			    ["u_velocities"].vec3(velocities[i])//, CAP)
			    ["u_birth_death"].vec2(birth_deaths[i])//, CAP)
			    ["u_uv_offset"].vec2(uv_offsets[i])//, CAP)
			    ["u_frame_dims"].vec2({1.f/6.f, 1.f})
			    ["u_time"].flt(time)
			    .set_camera(cam)
			    .template draw<GL_TRIANGLES>();
		}
	}
};

}