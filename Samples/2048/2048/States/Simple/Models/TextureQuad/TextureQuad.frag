#version 330 core

layout(location = 0) out vec4 out_color;

in vec2 uv_coords;

uniform sampler2D in_texture;

void main(void) {
	out_color = texture(in_texture, uv_coords);
}
