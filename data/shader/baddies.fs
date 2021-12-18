#version 410
in vec4 v_screen_pos;
in vec2 v_uv;

uniform sampler2D u_sprite_sheet;
uniform sampler2D u_armor_sheet;

uniform int  u_frame;
uniform vec2 u_frame_dims;
uniform float u_armor;
uniform float u_shield;

out vec4 color;

void main (void)
{
	float fog = min(1.0, 1.0/(0.1 * length(v_screen_pos.xyz)));
	vec4 sprite_color = texture(u_sprite_sheet, v_uv * u_frame_dims + vec2(u_frame_dims.x * u_frame, 0));
	vec4 armor_color = texture(u_armor_sheet, v_uv) * sprite_color.a;
	
	color = sprite_color * (1.0 - (armor_color.a * u_armor)) + armor_color * u_armor;
	color = mix(color, vec4(0.5, 0.5, 1.0, 1.0) * color.a, u_shield);
	color.rgb *= fog;


	if (color.a < 0.1) { discard; }
}