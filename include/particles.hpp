#pragma once
#include <unordered_map>

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
	~sprite() = default;

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

	vec<2> frame_uv_offset(unsigned frame)
	{
		return {frame_dims[0] * frame, 0};
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
	vec<3> positions[CAP] = {};
	float  scales[CAP] = {};
	float  alphas[CAP] = {};
	vec<3> velocities[CAP] = {};
	float  scale_vels[CAP] = {};
	float  alpha_vels[CAP] = {};
	vec<2> birth_deaths[CAP] = {};
	vec<2> uv_offsets[CAP] = {};
	int next = 0;

	particle_system()
	{
		for (unsigned i = 0; i < CAP; i++)
		{
			positions[i] = { 0, (float)i, 0 };
		}

	}

	void initialize(const sprite& s)
	{
		std::vector<particle_system::vertex> vertices;
		std::vector<uint32_t> indices;

		sprite_meta = s;

		for (uint16_t i = 0; i < 1; i++)
		{
			const auto s = 0.5f;
			auto n = vertices.size();
			vertices.push_back({{-s,-s, 0 }, {1, 1}, 0});
			vertices.push_back({{ s,-s, 0 }, {0, 1}, 0});
			vertices.push_back({{ s, s, 0 }, {0, 0}, 0});
			vertices.push_back({{-s, s, 0 }, {1, 0}, 0});

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


	void spawn(const vec<3>& position, float scale, float alpha, const vec<3>& velocity, float scale_vel, float alpha_vel, float birth_time, float death_time, const vec<2>& uv_offset)
	{
		positions[next] = position;
		scales[next] = scale;
		alphas[next] = alpha;
		velocities[next] = velocity;
		scale_vels[next] = scale_vel;
		alpha_vels[next] = alpha_vel;
		birth_deaths[next] = { birth_time, death_time };
		uv_offsets[next] = sprite_meta.frame_uv_offset(rand() % sprite_meta.frames);
		next = (next + 1) % CAP;
	}


	void draw(g::gfx::shader& shader, g::gfx::texture& sprite_sheet, g::game::camera& cam, float time)
	{
		for (unsigned i = 0; i < CAP; i++)
		{
			particles_mesh.using_shader(shader)
			    ["u_sprite_sheet"].texture(sprite_sheet)
			    ["u_positions"].vec3(positions[i])//, CAP)
			    ["u_scales"].flt(scales[i])//, CAP)
			    ["u_alphas"].flt(alphas[i])//, CAP)
			    ["u_velocities"].vec3(velocities[i])//, CAP)
			    ["u_scale_vels"].flt(scale_vels[i])
			    ["u_alpha_vels"].flt(alpha_vels[i])
			    ["u_birth_death"].vec2(birth_deaths[i])//, CAP)
			    ["u_uv_offset"].vec2(uv_offsets[i])//, CAP)
			    ["u_frame_dims"].vec2(sprite_meta.frame_dims)
			    ["u_time"].flt(time)
			    .set_camera(cam)
			    .template draw<GL_TRIANGLES>();
		}
	}
};

template <typename V>
struct gpu_particle_system : public g::game::updateable
{
	std::unordered_map<std::string, g::gfx::framebuffer> x; // state textures
	std::unordered_map<std::string, g::gfx::framebuffer> dx; // state first derivatives

	g::gfx::mesh<g::gfx::vertex::pos_uv_norm> quad_mesh;
	g::gfx::mesh<V> particle_mesh;

	inline unsigned capacity() const
	{
		return x[0].size[0] * x[0].size[1] * x[0].size[2];
	}

	// life:1, pos:3, scale:1, alpha:1,
	// nop:1,   vel:3, dscale:1 dalpha:1
	gpu_particle_system(std::function<void(std::vector<V>&, unsigned)> vertex_generator,
	                    unsigned state_size=6,
	                    unsigned capacity=1000)
	{
		quad_mesh = g::gfx::mesh_factory{}.plane();
		particle_mesh = g::gfx::mesh_factory::empty_mesh<V>();

		unsigned side = ceil(sqrt(capacity));
		unsigned texture_count = ceil(state_size / 4);

		for (unsigned i = 0; i < texture_count; i++)
		{
			auto x_tex = g::gfx::texture_factory{side, side}.components(4).type(GL_FLOAT).pixelated().create();
			auto dx_tex = g::gfx::texture_factory{side, side}.components(4).type(GL_FLOAT).pixelated().create();
			x["u_x" + std::to_string(i)] = g::gfx::framebuffer_factory{x_tex}.create();
			dx["u_dx" + std::to_string(i)] = g::gfx::framebuffer_factory{dx_tex}.create();
		}

		std::vector<V> vertices;
		for (unsigned i = 0; i < this->capacity(); i++)
		{
			vertex_generator(vertices, i);
		}

		particle_mesh.set_vertices(vertices);
	}

	void update(float dt, float t)
	{
		// render to dx textures if they change

		// render to state textures
		{
			// start rendering using dynamics shader
			auto chain = quad_mesh.using_shader(/*...*/)
			             ["u_dt"].flt(dt);

			for (auto& dst_name_fb_pair : x)
			{
				auto& x_fb = dst_name_fb_pair.second;

				x_fb.bind_as_target();
				glClear(GL_COLOR_BUFFER_BIT);

				for (auto& src_x : x)
				{
					chain.uniform(src_x.first).texture(src_x.second.color);
				}
				for (auto& src_dx : dx)
				{
					chain.uniform(src_dx.first).texture(src_dx.second.color);
				}

				chain.template draw<GL_TRIANGLE_FAN>();
				x_fb.unbind_as_target();
			}


		}

	}
};

}
