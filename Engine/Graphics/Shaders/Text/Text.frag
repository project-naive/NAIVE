#version 330 core

layout(location=0) out vec4 out_color;

in vec2 TexCoords;

uniform vec4 in_color;
uniform sampler2D Text_Texture;

void main(void) {
	out_color = in_color * vec4(1.0, 1.0, 1.0, texture(Text_Texture, TexCoords).r);
}

