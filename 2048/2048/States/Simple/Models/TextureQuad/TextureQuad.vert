#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoords;

layout(location = 0) out vec2 uv_coords;

uniform mat4 projection;

void main(void){
	gl_Position = projection * position;
	uv_coords = texCoords;
}



