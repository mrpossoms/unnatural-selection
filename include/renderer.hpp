#pragma once
#include "g.h"
#include "state.hpp"
#include "particles.hpp"

using mat4 = xmath::mat<4,4>;

namespace us
{

struct renderer
{
	unsigned level_hash = 0;
	g::gfx::mesh<g::gfx::vertex::pos_uv_norm> level_mesh;
	g::gfx::mesh<g::gfx::vertex::pos_uv_norm> billboard_mesh;
	std::unordered_map<std::string, sprite> sprites;
	std::unordered_map<std::string, nlohmann::json> json;
	// std::shared_ptr<us::particles::gpu_backend> ps = nullptr;
	std::shared_ptr<us::particles::gpu_mesh<>> ps_mesh = nullptr;

	std::map<std::string, std::shared_ptr<us::particles::gpu_backend>> particle_backends;

	void draw_onboarding(g::asset::store& assets, us::state& state)
	{
		if (state.onboarding_step == 0)
		{
			auto m = mat4::rotation({0, 1, 0}, M_PI / 2) * mat4::translation({(float)state.level->lymph_nodes[0][0] + 5, 2, (float)state.level->lymph_nodes[0][1]});
	        g::ui::layer tip(&assets, "basic_gui.vs+basic_font.fs", m);
	        tip.set_font("UbuntuMono-B.ttf");
	        tip.text("Hello nanobot, and welcome\nto the human body. You are\ntasked with cleaning up this\ninfection site from viral\nparticles.\n[E to continue]", state.player)
	        .set_camera(state.player)
	        .draw<GL_TRIANGLES>();
		}
		if (state.onboarding_step == 1)
		{
			auto m = mat4::rotation({0, 1, 0}, M_PI / 2) * mat4::translation({(float)state.level->lymph_nodes[0][0] + 5, 2, (float)state.level->lymph_nodes[0][1]});
	        g::ui::layer tip(&assets, "basic_gui.vs+basic_font.fs", m);
	        tip.set_font("UbuntuMono-B.ttf");
	        tip.text("To control your thrusters\nuse keys W A D S to\nselect weapons use keys\n1 2 3. To fire your selected\nweapon, press the left mouse button\n[E to continue]", state.player)
	        .set_camera(state.player)
	        .draw<GL_TRIANGLES>();
		}
		if (state.onboarding_step == 2)
		{
			auto m = mat4::rotation({0, 1, 0}, M_PI / 2) * mat4::translation({(float)state.level->lymph_nodes[0][0] + 5, 2, (float)state.level->lymph_nodes[0][1]});
	        g::ui::layer tip(&assets, "basic_gui.vs+basic_font.fs", m);
	        tip.set_font("UbuntuMono-B.ttf");
	        tip.text("Your objective is to defend\nthe purple lymph nodes such as\nthose infront of and behind\nyou from viruses\n[E to continue]", state.player)
	        .set_camera(state.player)
	        .draw<GL_TRIANGLES>();
		}
		if (state.onboarding_step == 3)
		{
			if (state.baddies.size() == 0)
			{
				us::baddie dummy;
				dummy.position = {(float)state.level->lymph_nodes[0][0] + 6, 1, (float)state.level->lymph_nodes[0][1]};
				dummy.reset(1);
				dummy.hp = 1;
				dummy.idle = true;
				dummy.shield = 0;
				dummy.armor = 0;
				state.baddies.push_back(dummy);
				std::cerr << std::to_string(dummy.hp) << std::endl;

				us::baddie shield_dummy;
				shield_dummy.position = {(float)state.level->lymph_nodes[0][0] + 6, 1, (float)state.level->lymph_nodes[0][1]-2};
				shield_dummy.reset(1);
				shield_dummy.hp = 1;
				shield_dummy.idle = true;
				shield_dummy.shield = 1;
				shield_dummy.armor = 0;
				state.baddies.push_back(shield_dummy);
				std::cerr << std::to_string(shield_dummy.hp) << std::endl;

				us::baddie armor_dummy;
				armor_dummy.position = {(float)state.level->lymph_nodes[0][0] + 6, 4, (float)state.level->lymph_nodes[0][1]+2};
				armor_dummy.reset(1);
				armor_dummy.hp = 1;
				armor_dummy.idle = true;
				armor_dummy.shield = 0;
				armor_dummy.armor = 1;
				state.baddies.push_back(armor_dummy);
				std::cerr << std::to_string(armor_dummy.hp) << std::endl;

			}

			auto m = mat4::rotation({0, 1, 0}, M_PI / 2) * mat4::translation({(float)state.level->lymph_nodes[0][0] + 5, 2, (float)state.level->lymph_nodes[0][1]});
	        g::ui::layer tip(&assets, "basic_gui.vs+basic_font.fs", m);
	        tip.set_font("UbuntuMono-B.ttf");
	        tip.text("Viruses are always evolving to\ncombat your tactics. On the\nleft is a virus that has evolved\narmor, in the center is an\nunprotected virus, on the right\nis one that has evolved shields.\n[E to continue]", state.player)
	        .set_camera(state.player)
	        .draw<GL_TRIANGLES>();
		}
		if (state.onboarding_step == 4)
		{
			auto m = mat4::rotation({0, 1, 0}, M_PI / 2) * mat4::translation({(float)state.level->lymph_nodes[0][0] + 5, 2, (float)state.level->lymph_nodes[0][1]});
	        g::ui::layer tip(&assets, "basic_gui.vs+basic_font.fs", m);
	        tip.set_font("UbuntuMono-B.ttf");
	        tip.text("Use your rifle [1] to counter\narmor. Your laser [2] to counter\nshields, and your shotgun to\nshred unprotected viruses\n[E to continue]", state.player)
	        .set_camera(state.player)
	        .draw<GL_TRIANGLES>();
		}
		if (state.onboarding_step == 5)
		{
			state.baddies.clear();
			state.onboarding_done = true;
		}
	}


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

	void draw(g::asset::store& assets, us::state& state, float dt)
	{
		if (state.level->hash != level_hash)
		{
			build_level_mesh(state.level);
		}

		if (nullptr == ps_mesh)
		{
			// state.gibs.initialize(get_sprite("particles"));
			// state.smoke.initialize(get_sprite("Smoke"));
			// state.chunks.initialize(get_sprite("chunks"));
			// ps = std::make_shared<us::particles::gpu_backend>(8, 1000);
			// ps_mesh = std::make_shared<us::particles::gpu_mesh<>>(*ps);
		}

		if (!billboard_mesh.is_initialized())
		{
			billboard_mesh = g::gfx::mesh_factory::plane();
		}

		// update and spawn particles
		for (auto& pq_kvp : state.particle_queue)
		{
			const auto& name = pq_kvp.first;
			auto& spawn_queue = pq_kvp.second;

			// check to see if the particle system has been initialized, if not create it
			const auto& itr = particle_backends.find(name);
			if (itr == particle_backends.end())
			{
				particle_backends[name] = std::make_shared<us::particles::gpu_backend>(8, 1000);

				if (nullptr == ps_mesh)
				{
					ps_mesh = std::make_shared<us::particles::gpu_mesh<>>(*particle_backends[name]);
				}
			}

			auto& backend = particle_backends[name];

			// spawn any particles that are in line
			while (spawn_queue.size() > 0)
			{
				backend->spawn(spawn_queue[spawn_queue.size()-1].v,
					      spawn_queue[spawn_queue.size()-1].v + backend->state_size);
				spawn_queue.pop_back();
			}

			backend->update(dt, state.time);
		}

		// ps->update(dt, state.time);

		// while(state.particle_spawn_queue.size() > 0)
		// {

		// 	ps->spawn(state.particle_spawn_queue[state.particle_spawn_queue.size()-1].v,
		// 		      state.particle_spawn_queue[state.particle_spawn_queue.size()-1].v + 8);
		// 	state.particle_spawn_queue.pop_back();
		// }

		// while(state.particle_spawn_queue.size() > 0)
		// {

		// 	ps->spawn(state.particle_spawn_queue[state.particle_spawn_queue.size()-1].v,
		// 		      state.particle_spawn_queue[state.particle_spawn_queue.size()-1].v + 6);
		// 	state.particle_spawn_queue.pop_back();
		// }

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		state.player.aspect_ratio(g::gfx::aspect());

		level_mesh.using_shader(assets.shader("level.vs+level.fs"))
		    ["u_model"].mat4(mat4::I())
		    ["u_floor"].texture(assets.tex("floor_0.repeating.png"))
		    ["u_roof"].texture(assets.tex("roof.repeating.png"))
		    ["u_wall"].texture(assets.tex("wall_0.repeating.png"))
		    .set_camera(state.player)
		    .draw<GL_TRIANGLES>();

		// render pustules
		for (auto& lymph_node : state.level->lymph_nodes)
		{
			const std::string nodes_strs[] = {
				"node_damaged_2.obj", "node_damaged_1.obj", "node_damaged_0.obj", "node.obj",
			};

			auto lymph_node_hp = state.level->cells[lymph_node[0]][lymph_node[1]].lymph_node_hp;
			if (lymph_node_hp > 0)
			{
				auto& node_mesh = assets.geo(nodes_strs[std::min<unsigned>(3, lymph_node_hp / 25)]);

				node_mesh.using_shader(assets.shader("level.vs+model.fs"))
				    ["u_model"].mat4(mat4::translation(vec<3>{(float)lymph_node[0], 0, (float)lymph_node[1]}))
				    ["u_texture"].texture(assets.tex("nodeTexture.png"))
				    .set_camera(state.player)
				    // .draw<GL_POINTS>();
				    .draw<GL_TRIANGLES>();
			}
		}


		// render spawners
		auto& spawner_mesh = assets.geo("spawner.obj");
		for (auto& point : state.level->spawn_points)
		{
			spawner_mesh.using_shader(assets.shader("level.vs+model.fs"))
			    ["u_model"].mat4(mat4::translation(vec<3>{(float)point[0], 6, (float)point[1]}))
			    ["u_texture"].texture(assets.tex("spawner.png"))
			    .set_camera(state.player)
			    .draw<GL_TRIANGLES>();
		}
		// glDisable(GL_DEPTH_TEST);

		// draw the baddies
		// glDisable(GL_DEPTH_TEST);
		for (unsigned i = 0; i < state.baddies.size(); i++)
		{
			auto& baddie = state.baddies[i];

			if (baddie.hp <= 0) { continue; }

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

			billboard_mesh.using_shader(assets.shader("billboard.vs+baddies.fs"))
				["u_position"].vec3(baddie.position)
				["u_sprite_sheet"].texture(assets.tex(imgs[i%3]))
				["u_armor_sheet"].texture(assets.tex("armor.png"))
				["u_frame_dims"].vec2(meta.frame_dims)
				["u_shield"].flt(baddie.shield)
				["u_armor"].flt(baddie.armor)
				["u_frame"].int1(static_cast<int>((state.time * meta.duration) + (i * 10)) % meta.frames)
				.set_camera(state.player)
				.draw<GL_TRIANGLE_FAN>();
		}
		// glEnable(GL_DEPTH_TEST);


		// projectiles
		for (unsigned i = 0; i < state.projectiles.size(); i++)
		{
			us::projectile& projectile = state.projectiles[i];
			const std::string imgs[][4] = {
				{"bullet_0.png", "bullet_1.png", "bullet_2.png", "bullet_3.png"},
				{"laser.png", "laser.png", "laser.png", "laser.png"},
				{"bullet_0.png", "bullet_1.png", "bullet_2.png", "bullet_3.png"},
			};

			// if (projectile.type == us::projectile::category::laser) glBlendFunc(GL_ONE, GL_ONE);
			billboard_mesh.using_shader(assets.shader("billboard.vs+animated_sprite.fs"))
				["u_position"].vec3(projectile.position)
				["u_sprite_sheet"].texture(assets.tex(imgs[projectile.type][i%4]))
				["u_frame_dims"].vec2({1, 1})
				["u_frame"].int1(0)
				.set_camera(state.player)
				.draw<GL_TRIANGLE_FAN>();
			// if (projectile.type == us::projectile::category::laser) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		// draw particles
		for (auto& pb_kvp : particle_backends)
		{
			if (pb_kvp.first == "smoke") { glDepthMask(GL_FALSE); }
			ps_mesh->mesh.using_shader(assets.shader("gpu_particle.vs+gpu_particle.fs"))
			.set_camera(state.player)
			["u_sprite_sheet"].texture(assets.tex(pb_kvp.first + ".png"))
			// // ["u_uv_offset"].vec2(uv_offsets[i])//, CAP)
			// ["u_frame_dims"].vec2(vec<2>{1, 1})//get_sprite(pb_kvp.first).frame_dims)
			// ["u_uv_offset"].vec2(uv_offsets[i])//, CAP)
			["u_frame_dims"].vec2(get_sprite(pb_kvp.first).frame_dims)
			["u_x0"].texture(pb_kvp.second->x[0].color)
			["u_x1"].texture(pb_kvp.second->x[1].color)
		    .draw<GL_TRIANGLES>();
		    if (pb_kvp.first == "smoke") { glDepthMask(GL_TRUE); }
		}

/*
		state.gibs.draw(assets.shader("particle.vs+particle.fs"), assets.tex("particles.png"), state.player, state.time);
		state.chunks.draw(assets.shader("particle.vs+particle.fs"), assets.tex("chunks.png"), state.player, state.time);
*/
		// glDisable(GL_DEPTH_TEST);
		// ps_mesh->mesh.using_shader(assets.shader("gpu_particle.vs+gpu_particle.fs"))
		// .set_camera(state.player)
		// ["u_sprite_sheet"].texture(assets.tex("particles.png"))
		// // ["u_uv_offset"].vec2(uv_offsets[i])//, CAP)
		// ["u_frame_dims"].vec2(get_sprite("particles").frame_dims)
		// ["u_x0"].texture(ps->x[0].color)
		// ["u_x1"].texture(ps->x[1].color)
	 //    .draw<GL_TRIANGLES>();
		// glEnable(GL_DEPTH_TEST);

		// glDepthMask(GL_FALSE);
		// //state.smoke.draw(assets.shader("particle.vs+particle.fs"), assets.tex("smoke.png"), state.player, state.time);
		// glDepthMask(GL_TRUE);

		if (!state.onboarding_done)
		{draw_onboarding(assets, state);}

		glDisable(GL_DEPTH_TEST);

		const std::string gun_models[3] = {
			"gun.obj", "lazer.obj", "Shotty.obj"
		};

		const std::string gun_tex[3] = {
			"gun_carbine.png", "gun_laser.png", "gun_shotty.png"
		};

		const std::string gun_json[3] = {
			"gun_carbine", "gun_laser", "gun_shotty"
		};

		auto& gun_sprite = get_sprite(gun_json[state.player.selected_weapon]);
		assets.geo("gun_plane.obj").using_shader(assets.shader("level.vs+animated_sprite.fs"))
		    ["u_sprite_sheet"].texture(assets.tex(gun_tex[state.player.selected_weapon]))
			// ["u_texture"].texture(assets.tex(gun_tex[state.player.selected_weapon]))
		    ["u_model"].mat4(mat4::scale({-3, 3, -3}) * mat4::translation({-1.25, -0.5, -1}))
		    ["u_view"].mat4(mat4::I())
		    ["u_proj"].mat4(state.player.projection())
			["u_frame_dims"].vec2(gun_sprite.frame_dims)
			["u_frame"].int1(std::min<int>(2, state.player.gun_shoot * 3))//static_cast<int>((state.time * get_sprite("gun_carbine").duration)) % get_sprite("gun_carbine").frames)
		    .draw<GL_TRIANGLES>();

		const std::string gun_reticles[3] = {
			"Ret_1.png", "Ret_2.png", "Ret_3.png"
		};

        g::ui::layer root(&assets, "basic_gui.vs+basic_gui.fs");
        root.set_font("UbuntuMono-B.ttf");

        if (state.onboarding_done)
        {
	        auto wave_msg = "more coming in " + std::to_string((int)state.wave.count_down);

	        if (state.wave.count_down > 28)
	        {
	        	wave_msg = "wave " + std::to_string(state.wave.number);
	        }

	        if (state.level->living_lymph_nodes().size() == 0)
	        {
	        	wave_msg = "GAME OVER";
	        }

	        auto wave_text = root.child({-0.2, 0.2, 1}, {0, 0.9, -1.f}).set_shaders("basic_gui.vs+basic_font.fs");
	        wave_text.text(wave_msg, state.player)
	        ["u_view"].mat4(mat4::I())
	        ["u_proj"].mat4(state.player.projection())
	        .draw<GL_TRIANGLES>();
        }

        auto reticle = root.child({0.05, 0.05}, {0, 0, -1});
        reticle.using_shader()
        ["u_view"].mat4(mat4::I())
        ["u_proj"].mat4(state.player.projection())
        ["u_texture"].texture(assets.tex(gun_reticles[state.player.selected_weapon]))
        ["u_border_thickness"].flt(0)
        .draw_tri_fan();

		// auto x0 = root.child({ 0.2, 0.2 }, { -0.5, 0, -1 });
		// x0.using_shader()
		// 	["u_view"].mat4(mat4::I())
		// 	["u_proj"].mat4(state.player.projection())
		// 	["u_texture"].texture(ps->x[0].color)
		// 	//["u_color"].vec4({0, 0, 0, 1})
		// 	["u_border_thickness"].flt(0.1)
		// 	.draw_tri_fan();

		// auto x1 = root.child({ 0.2, 0.2 }, { -1, 0, -1 });
		// x1.using_shader()
		// 	["u_view"].mat4(mat4::I())
		// 	["u_proj"].mat4(state.player.projection())
		// 	["u_texture"].texture(ps->x[1].color)
		// 	["u_border_thickness"].flt(0.1)
		// 	.draw_tri_fan();

        auto gun_ui = root.child(vec<2>{0.3, 0.1} * 1.1f, {1.5, -1.5f, -1});
        // gun_ui.using_shader()
        // ["u_view"].mat4(mat4::I())
        // ["u_proj"].mat4(state.player.projection())
        // ["u_border_thickness"].flt(0.1)
        // ["u_border_color"].vec4({1, 1, 1, 1})
        // .draw_tri_fan();

        auto carbine_icon = gun_ui.child(vec<2>{0.66f, -1.0f} * (1 + (state.player.selected_weapon == 0) * 0.33f), {0.5f, 0, -1});
        carbine_icon.using_shader()
        ["u_view"].mat4(mat4::I())
        ["u_proj"].mat4(state.player.projection())
        ["u_texture"].texture(assets.tex("carbine_ui.png"))
        .draw_tri_fan();

        auto laser_icon = gun_ui.child(vec<2>{0.66f, -1.0f} * (1 + (state.player.selected_weapon == 1) * 0.33f), {0.0f, 0, -1});
        laser_icon.using_shader()
        ["u_view"].mat4(mat4::I())
        ["u_proj"].mat4(state.player.projection())
        ["u_texture"].texture(assets.tex("laser2_ui.png"))
        .draw_tri_fan();

        auto shotty_icon = gun_ui.child(vec<2>{0.66f, -1.0f} * (1 + (state.player.selected_weapon == 2) * 0.33f), {-0.5f, 0, -1});
        shotty_icon.using_shader()
        ["u_view"].mat4(mat4::I())
        ["u_proj"].mat4(state.player.projection())
        ["u_texture"].texture(assets.tex("shotty_ui.png"))
        .draw_tri_fan();

		glEnable(GL_DEPTH_TEST);
	}
};

}
