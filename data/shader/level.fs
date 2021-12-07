#version 410
in vec4 v_screen_pos;
in vec3 v_normal;
in vec2 v_uv;

uniform sampler2D u_texture;

out vec4 color;

void main (void)
{
	color = vec4(v_normal * 0.5 + vec3(0.5), 1.0);
	// color = texture(u_texture, vec2(1.0 - v_uv.x, v_uv.y));
}