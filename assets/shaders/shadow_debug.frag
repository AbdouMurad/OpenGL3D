#version 330 core

//debug view: one shadow map layer as grey. dark = close to the light, white = far / nothing there
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2DArray u_shadowMap;
uniform int u_layer;

void main() {
	float depth = texture(u_shadowMap, vec3(TexCoord, u_layer)).r;
	FragColor = vec4(vec3(depth), 1.0);
}
