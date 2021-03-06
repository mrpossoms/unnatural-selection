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
	vec3 position = x0.yzw;
	float scale = x1.x;
	v_alpha = x1.y;
	float uv_offset = x1.z;
	float rotation = x1.w;

	float c = cos(rotation);
	float s = sin(rotation);
	mat3 rot = mat3(
		c, -s, 0,
		s,  c, 0,
		0,  0, 1
	);

	vec4 v_world_pos = vec4(position, 1.0);
	v_screen_pos = (u_proj * u_view * v_world_pos) + vec4((rot * a_position) * scale, 0.0);
	gl_Position = v_screen_pos;

	v_uv = (a_uv * u_frame_dims) + vec2(uv_offset, 0.0);// + u_uv_offset;
}
