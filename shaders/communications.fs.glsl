#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float notificationTimer;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	float default_notification_timer = 500.f;
	vec4 mod = vec4(-1.0f, 0.5f, 0.5f, 0.0f) * notificationTimer/default_notification_timer;
	color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y)) + mod;
}
