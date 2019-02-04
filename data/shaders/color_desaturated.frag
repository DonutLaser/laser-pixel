#version 330 core

out vec4 result_color;

uniform vec4 color;

void main () {
	float avg = (color.r + color.g + color.b) / 3;
	result_color.rgb = vec3 (avg);
	result_color.a = color.a;
}
