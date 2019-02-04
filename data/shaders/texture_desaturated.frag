#version 330 core

in vec2 uv;

out vec4 result_color;

uniform sampler2D image;
uniform vec4 color;

void main () {
	vec4 final = color * texture (image, uv);
	float avg = (final.r + final.g + final.b) / 3;
	
	result_color.rgb = vec3 (avg);
	result_color.a = final.a;
}