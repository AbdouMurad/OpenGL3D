#version 330 core

out vec4 FragColor;

in vec3 crntPos;
in vec3 Normal;
in vec3 color;
in vec2 texCoord;

uniform vec3 camPos;

uniform sampler2D tex;

void main() {
	//FragColor = texture(tex, texCoord);
	FragColor = vec4(1,0,0,1);
}