#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;

uniform mat4 u_projection;

out vec4 color;

void main() {
	color = aColor;
	gl_Position = u_projection * vec4(aPos, 0.0, 1.0);
}
