#version 330 core

//shadow pass: only positions matter. the GPU writes each pixel's depth (distance from the light) by itself
layout (location = 0) in vec3 aPos;

uniform mat4 u_lightMatrix; //light projection * light view (the light's "camMatrix")
uniform mat4 model;

void main() {
	gl_Position = u_lightMatrix * model * vec4(aPos, 1.0);
}
