#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 3) in vec2 aTex;

out vec3 crntPos;
out vec2 texCoord;

uniform mat4 camMatrix;
uniform mat4 model;

void main() {
	texCoord = aTex;
	crntPos = vec3(model * vec4(aPos, 1.0));
	gl_Position = camMatrix * vec4(crntPos, 1.0);
}