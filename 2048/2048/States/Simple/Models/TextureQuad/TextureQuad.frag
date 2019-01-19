#version 330 core

layout(location = 0) in vec2 uv_coords;

layout(location = 0) out vec4 out_color;

uniform sampler2D tex;

void main(void) {
	out_color = vec4(texture(tex, uv_coords).rgba);
}
