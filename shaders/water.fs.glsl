#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float darken_screen_factor;
uniform bool is_paused;

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec2 distort(vec2 uv) 
{
	return uv;
}

vec4 color_shift(vec4 in_color) 
{
	if (!is_paused) {
		in_color.xyz = in_color.xyz * vec3(0.1 * sin(-0.1 * time) + 0.6, 0.9, 0.9);
	}
	return in_color;
}

vec4 fade_color(vec4 in_color) 
{
	if (darken_screen_factor > 0)
		in_color -= darken_screen_factor * vec4(1.0, 1.0, 1.0, 0);
	return in_color;
}

void main()
{
	vec2 coord = distort(texcoord);

    vec4 in_color = texture(screen_texture, coord);
    color = color_shift(in_color);
    color = fade_color(color);
}