#version 410
in vec2 v_uv;

out vec4 color;

void main (void)
{
	color = vec4(v_uv, 0.0, 1.0);
}