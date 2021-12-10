#version 410
in vec4 v_screen_pos;
in vec3 v_normal;
in vec2 v_uv;

uniform sampler2D u_floor;
uniform sampler2D u_roof;
uniform sampler2D u_wall;

out vec4 color;

void main (void)
{
	// color = vec4(v_normal * 0.5 + vec3(0.5), 1.0);
	
	vec4 floor_textel = texture(u_floor, vec2(1.0 - v_uv.x, v_uv.y));
	vec4 roof_textel = texture(u_roof, vec2(1.0 - v_uv.x, v_uv.y));
	vec4 wall_textel = texture(u_wall, vec2(1.0 - v_uv.x, v_uv.y));

	float wp = 1.f - abs(dot(v_normal, vec3(0, 1, 0)));
	float fp = max(0, dot(v_normal, vec3(0, 1, 0)));
	float rp = max(0, dot(v_normal, vec3(0, -1, 0)));

	float fog = min(1.0, 1.0/(0.1 * v_screen_pos.z));

	color = (wall_textel * wp + floor_textel * fp + roof_textel * rp) * vec4(vec3(fog), 1.0);
}