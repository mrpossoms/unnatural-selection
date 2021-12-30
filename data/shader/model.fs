#ifdef GL_ES
precision mediump float;
#endif


in vec4 v_screen_pos;
in vec3 v_normal;
in vec2 v_uv;

uniform sampler2D u_texture;

out vec4 color;

void main (void)
{
	float fog = min(1.0, 1.0/(sqrt(length(v_screen_pos.xyz))));

	color = texture(u_texture, v_uv * vec2(1, -1) + vec2(0, 1)) * vec4(vec3(fog), 1.0);
}
