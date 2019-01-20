#version 330 core

layout(location = 0) in vec2 uv_coords;

layout(location = 0) out vec4 out_color;

uniform sampler2D in_texture;

void main(void) {
	out_color = vec4(texture(in_texture, uv_coords).rgba);
}
