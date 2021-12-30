#version 300 es
//#version 410

#ifdef GL_ES
precision mediump float;
#endif


in vec4 v_screen_pos;
in vec2 v_uv;
in float v_alpha;

uniform sampler2D u_sprite_sheet;

out vec4 color;

void main (void)
{
	float fog = min(1.0, 1.0/(0.2 * length(v_screen_pos.xyz)));

	color = texture(u_sprite_sheet, v_uv);
	color.rgb *= fog;
	color.a *= v_alpha;

	// color = vec4(0.0, 0.0, 1.0, 1.0);

	if (color.a < 0.01) { discard; }
}