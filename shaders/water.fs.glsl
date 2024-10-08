#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float darken_screen_factor;

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec2 distort(vec2 uv) 
{
	float distort_x_intensity = 0.002 * (-((2 * uv.x - 1) * (2 * uv.x - 1)) + 1);
	float distort_y_intensity = 0.002 * (-((2 * uv.y - 1) * (2 * uv.y - 1)) + 1);

	float x_offset_oscillator = distort_x_intensity * sin(10 * uv.x - 0.1 * time);
	float y_offset_oscillator = distort_y_intensity * sin(10 * uv.y - 0.1 * time);

	uv = vec2(uv.x + x_offset_oscillator, uv.y + y_offset_oscillator);
	return uv;
}

vec4 color_shift(vec4 in_color) 
{
	return in_color;
}

vec4 fade_color(vec4 in_color) 
{
	if (darken_screen_factor > 0)
		in_color -= darken_screen_factor * vec4(0.8, 0.8, 0.8, 0);
	return in_color;
}

void main()
{
	vec2 coord = distort(texcoord);

    vec4 in_color = texture(screen_texture, coord);
    color = color_shift(in_color);
    color = fade_color(color);
}