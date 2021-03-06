#ifdef GL_ES
precision mediump float;
#endif


in vec4 v_screen_pos;
in vec2 v_uv;
// in float v_alpha;

uniform vec2  u_birth_death;
uniform float u_alphas;
uniform float u_alpha_vels;

uniform float u_time;
uniform sampler2D u_sprite_sheet;

out vec4 color;

void main (void)
{
	float lived = u_time - u_birth_death.x;
	float fog = min(1.0, 1.0/(0.2 * length(v_screen_pos.xyz)));
	float alpha_t = u_alphas + (u_alpha_vels * lived);

	color = texture(u_sprite_sheet, v_uv);
	color.rgb *= fog;
	color.a *= alpha_t;

	if (color.a < 0.01) { discard; }
}
