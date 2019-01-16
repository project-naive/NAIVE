#version 330 core

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 input_color;
layout(location = 2) in vec4 uv_coords;

uniform mat4 projection;
out vec4 uv_cordinates;
out vec4 frag_color;

void main(void) {
	uv_cordinates = uv_coords;
	frag_color = input_color;
	gl_Position = projection * in_position;
}


