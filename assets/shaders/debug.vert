#version 330 core

layout (location = 0) in vec3 aPos;

unifrom mat4 u_camMatrix;


void main() {
	gl_position = u_camMatrix * vec4(aPos, 1.0);
}