#version 410
#ifdef GL_ES
precision mediump float;
#endif

in vec3 a_position;
in vec2 a_uv;
in int a_index;

uniform vec3 u_positions[128];
uniform vec3 u_velocities[128];
uniform vec2 u_birth_death[128];
uniform vec2 u_uv_offset[128];

uniform vec2 u_frame_dims;
uniform float u_time;

uniform mat4 u_view;
uniform mat4 u_proj;

// uniform mat4 u_light_view;
// uniform mat4 u_light_proj;

out vec4 v_screen_pos;
out vec3 v_normal;
out vec2 v_uv;

void main (void)
{
	float lived = u_time - u_birth_death[a_index].x;

	vec4 v_world_pos = vec4(u_positions[a_index] + (u_velocities[a_index] * lived), 1.0);
	v_screen_pos = u_proj * ((u_view * v_world_pos) + vec4(a_position, 0.0));
	gl_Position = v_screen_pos;

	v_uv = (a_uv * u_frame_dims) + u_uv_offset[a_index];
}