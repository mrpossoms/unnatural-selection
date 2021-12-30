#ifdef GL_ES
precision mediump float;
#endif

in vec4 v_screen_pos;
in vec2 v_uv;

uniform sampler2D u_sprite_sheet;

uniform int  u_frame;
uniform vec2 u_frame_dims;

out vec4 color;

void main (void)
{
	float fog = min(1.0, 1.0/(0.1 * length(v_screen_pos.xyz)));
	color = texture(u_sprite_sheet, (v_uv * vec2(1, -1) + vec2(0, 1))  * u_frame_dims + vec2(u_frame_dims.x * float(u_frame), 0));
	color.rgb *= fog;

	if (color.a < 0.1) { discard; }
}
