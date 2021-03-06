#ifdef GL_ES
precision mediump float;
#endif

in vec3 a_position;
in vec2 a_uv;
in vec3 a_normal;

uniform vec3 u_position;
uniform mat4 u_view;
uniform mat4 u_proj;

// uniform mat4 u_light_view;
// uniform mat4 u_light_proj;

out vec4 v_screen_pos;
out vec3 v_normal;
out vec2 v_uv;

void main (void)
{
	vec4 v_world_pos = vec4(u_position, 1.0);
	v_screen_pos = u_proj * ((u_view * v_world_pos) + vec4(a_position, 0.0));
	gl_Position = v_screen_pos;

	v_uv = a_uv;

    // mat3 model_rot = mat3(normalize(u_model[0].xyz), normalize(u_model[1].xyz), normalize(u_model[2].xyz));
    v_normal = normalize(a_normal);
	// v_light_proj_pos = u_light_proj * u_light_view * v_world_pos;
	// v_light_proj_pos /= v_light_proj_pos.w;
	// v_light_proj_pos = (v_light_proj_pos + 1.0) / 2.0;
}
