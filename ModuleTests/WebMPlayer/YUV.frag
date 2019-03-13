#version 330 core

layout(location=0) out vec4 out_color;

in vec2 Tex_UV;

uniform sampler2D Y_component;
uniform sampler2D U_component;
uniform sampler2D V_component;

mat3x3 transform=mat3x3(1.0f, 1.0f, 1.0f,
						0.0f, -0.21482f, 2.12798f,
						1.28033f, -0.38059f, 0.0f);


void main(void){
out_color = vec4(texture(Y_component, Tex_UV).r,texture(U_component, Tex_UV).r,  texture(V_component, Tex_UV).r, 1.0f);
}

