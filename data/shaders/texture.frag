#version 330 core

in vec2 uv;

out vec4 result_color;

uniform sampler2D image;
uniform vec4 color;

void main () {
	result_color = color * texture (image, uv);
}