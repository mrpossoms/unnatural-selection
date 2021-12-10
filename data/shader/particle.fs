#version 410
in vec4 v_screen_pos;
in vec2 v_uv;

uniform sampler2D u_sprite_sheet;

out vec4 color;

void main (void)
{
	float fog = min(1.0, 1.0/(0.2 * length(v_screen_pos.xyz)));
	color = texture(u_sprite_sheet, v_uv);
	color.rgb *= fog;

	if (color.a < 0.1) { discard; }
}