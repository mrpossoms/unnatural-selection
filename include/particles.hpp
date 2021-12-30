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

namespace particles
{

template <size_t STATE_SIZE=6>
struct gpu_backend : public g::game::updateable
{
	std::vector<g::gfx::framebuffer> x; // state textures
	std::vector<g::gfx::framebuffer> dx; // state first derivatives

	g::gfx::mesh<g::gfx::vertex::pos_uv_norm> quad_mesh;

	g::gfx::shader dynamics_shader;
	g::gfx::shader spawn_shader;

	vec<2> particle_stride;

	unsigned next_particle = 0;


	inline unsigned capacity() const
	{
		return x[0].size[0] * x[0].size[1] * x[0].size[2];
	}

	// life:1, pos:3, scale:1, alpha:1,
	// nop:1,   vel:3, dscale:1 dalpha:1
	gpu_particle_system(unsigned capacity=1000)
	{
		const std::string quad_vs = "#version 300 es\n"
		"precision mediump float;"
		"in vec3 a_position;"
		"in vec2 a_uv;"
		"out vec2 v_uv;"
		""
		"void main (void)"
		"{"
		"	v_uv = a_uv;"
		"}";

		const std::string dynamics_fs = "#version 300 es\n"
		"precision mediump float;"
		"in vec2 v_uv;"
		"out vec4 color;"
		"uniform sampler2D u_x;"
		"uniform sampler2D u_dx;"
		"uniform float u_dt;"
		""
		"void main (void)"
		"{"
		"	color = texture(u_x, v_uv) + texture(u_dx, v_uv) * u_dt;"
		"}";

		const std::string spawn_fs = "#version 300 es\n"
		"precision mediump float;"
		"in vec2 v_uv;"
		"uniform vec4 u_x0;"
		"out vec4 color;"
		""
		"void main (void)"
		"{"
		"	color = u_x0;"
		"}";

		quad_mesh = g::gfx::mesh_factory{}.plane();

		dynamics_shader = g::gfx::shader_factory{}.template add_src<GL_VERTEX_SHADER>(quad_vs)
		                                          .template add_src<GL_FRAGMENT_SHADER>(dynamics_fs)
		                                          .create();

		spawn_shader = g::gfx::shader_factory{}.template add_src<GL_VERTEX_SHADER>(quad_vs)
		                                       .template add_src<GL_FRAGMENT_SHADER>(spawn_fs)
		                                       .create();

		unsigned side = ceil(sqrt(capacity));
		unsigned texture_count = ceil(STATE_SIZE / 4);

		particle_stride = vec<2>{side, side} / 1.f;

		for (unsigned i = 0; i < texture_count; i++)
		{
			auto x_tex = g::gfx::texture_factory{side, side}.components(4).type(GL_FLOAT).pixelated().create();
			auto dx_tex = g::gfx::texture_factory{side, side}.components(4).type(GL_FLOAT).pixelated().create();
			x.push_back(g::gfx::framebuffer_factory{x_tex}.create());
			dx.push_back(g::gfx::framebuffer_factory{dx_tex}.create());
		}
	}

	void update(float dt, float t)
	{
		// render to dx textures if they change

		// render to state textures
		{
			// start rendering using dynamics shader
			auto chain = quad_mesh.using_shader(dynamics_shader)
			             ["u_dt"].flt(dt);

			for (unsigned i = 0; i < x.size(); i++)
			{
				x[i].bind_as_target();
				glClear(GL_COLOR_BUFFER_BIT);
				chain["u_x"].texture(x[i].color);
				chain["u_dx"].texture(dx[i].color);
				chain.template draw<GL_TRIANGLE_FAN>();
				x[i].unbind_as_target();
			}
		}
	}

	void spawn(const vec<STATE_SIZE> x_0)
	{
		auto chain = quad_mesh.using_shader(spawn_shader)
		int offset = 0;
		for (unsigned i = 0; i < x.size(); i++)
		{
			x[i].bind_as_target();
			glClear(GL_COLOR_BUFFER_BIT);
			chain["u_x0"].vec4(vec<4>{x_0.v + std::min<int>(STATE_SIZE, i * 4)});
			chain.template draw<GL_TRIANGLE_FAN>();
			x[i].unbind_as_target();
		}	

		next_particle = (next_particle + 1) % STATE_SIZE;
	}
};

} // namespace particles

}
