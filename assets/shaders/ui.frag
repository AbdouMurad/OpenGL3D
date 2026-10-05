#version 330 core

in vec4 color;

out vec4 FragColor;

void main() {
	//colors come in as sRGB, the sRGB framebuffer converts linear output back
	FragColor = vec4(pow(color.rgb, vec3(2.2)), color.a);
}
