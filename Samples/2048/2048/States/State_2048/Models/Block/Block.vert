#version 330 core

layout(location = 0) in vec4 CoordAndUV;

out vec2 uv;

uniform mat4 projection;

void main(void){
	gl_Position = projection * vec4(CoordAndUV.xy, 0.0f, 1.0f);
	uv = CoordAndUV.zw;
}
