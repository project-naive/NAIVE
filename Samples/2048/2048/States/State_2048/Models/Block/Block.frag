#version 330 core

layout(location = 0) out vec4 out_color;

in vec2 uv;

uniform float radius;
uniform vec4 color;

void main(void){
	vec2 moved = vec2(abs(uv.x),abs(uv.y)) - vec2(1.0f-radius, 1.0f-radius);
	float alpha = float(moved.x<0.0f||moved.y<0.0f||length(moved)<radius);
	out_color = vec4(color.rgb, color.a*alpha);
}

