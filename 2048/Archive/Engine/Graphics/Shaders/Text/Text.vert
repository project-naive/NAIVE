#version 330 core

layout(location = 0) in vec4 vertex;

out vec4 gl_Position;
out vec2 TexCoords;

uniform mat4 projection;

void main(void){
	TexCoords = vertex.zw;
	gl_Position = projection * vec4(vertex.xy, 1.0, 1.0);
}

