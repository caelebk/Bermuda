#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float time;
uniform bool is_low_oxygen;

// Output color
layout(location = 0) out  vec4 color;

// Inspired By: https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/a-brief-introduction-to-lerp-r4954/
float lerp (float initial, float final, float t) {
	return initial * (1 - t) + final * t;
}

void main()
{
	vec4 oxygen_bar_color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));

	if (is_low_oxygen) {
		oxygen_bar_color.x = lerp(oxygen_bar_color.x, 1.0, (0.5 * sin(0.7 * time) + 0.5));
		oxygen_bar_color.y = lerp(oxygen_bar_color.y, 0.25, (0.5 * sin(0.7 * time) + 0.5));
		oxygen_bar_color.z = lerp(oxygen_bar_color.z, 0.25, (0.5 * sin(0.7 * time) + 0.5));
	}

	color = oxygen_bar_color;
}
