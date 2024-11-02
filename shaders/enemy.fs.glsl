#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float damageTimer;
uniform bool stunned;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	float default_damage_timer = 500.f;
	vec4 mod = stunned ? vec4(0.0f,0.0f,0.7f,0.0f) : vec4(1.0f,0.0f,0.0f,0.0f) * damageTimer/default_damage_timer;
	color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y)) + mod;
}
