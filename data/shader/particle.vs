#ifdef GL_ES
precision mediump float;
#endif

in vec3 a_position;
in vec2 a_uv;
in int a_index;

uniform vec3  u_positions;
uniform float u_scales;
uniform vec3  u_velocities;
uniform float u_scale_vels;
uniform vec2  u_birth_death;
uniform vec2  u_uv_offset;

uniform vec2 u_frame_dims;
uniform float u_time;

uniform mat4 u_view;
uniform mat4 u_proj;

// uniform mat4 u_light_view;
// uniform mat4 u_light_proj;

out vec4 v_screen_pos;
out vec2 v_uv;

void main (void)
{
	float lived = u_time - u_birth_death.x;

	vec3 position_t = u_positions + (u_velocities * lived);
	float scale_t = u_scales + (u_scale_vels * lived);

	vec4 v_world_pos = vec4(position_t, 1.0);
	v_screen_pos = (u_proj * u_view * v_world_pos) + vec4(a_position * scale_t, 0.0);
	gl_Position = v_screen_pos;

	v_uv = (a_uv * u_frame_dims) + u_uv_offset;
}
