#version 300 es
//#version 410

#ifdef GL_ES
precision highp float;
#endif

in vec3 a_position;
in vec2 a_uv;
in vec2 a_index;

uniform sampler2D u_x0;
uniform sampler2D u_x1; 

uniform vec2 u_frame_dims;
uniform mat4 u_view;
uniform mat4 u_proj;

// uniform mat4 u_light_view;
// uniform mat4 u_light_proj;

out vec4 v_screen_pos;
out vec2 v_uv;
out float v_alpha;

void main (void)
{
	vec4 x0 = texture(u_x0, a_index);
	vec4 x1 = texture(u_x1, a_index);

	float life = x0.x;
	vec3 position = x0.xyz * 1.0;
	float scale = 1.0;//x1.x;
	float alpha = x1.y;

	vec4 v_world_pos = vec4(position, 1.0);
	v_screen_pos = (u_proj * u_view * v_world_pos) + vec4(a_position * scale, 0.0);
	gl_Position = v_screen_pos;

	v_uv = (a_uv * u_frame_dims);// + u_uv_offset;
}