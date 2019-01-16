#version 330 core

layout(location=0) out vec4 out_color;

in vec4 frag_color;
in vec4 uv_cordinates;

void main(void){
	float dist = distance(vec4(0.0,0.0,0.0,1.0), vec4(uv_cordinates.xyz, 1.0));
	out_color = frag_color * (dist > 1 ? 0.0 : 1.0);
}


