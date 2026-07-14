#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform vec4 u_baseColor;

uniform bool u_hasBaseColorTex;
uniform sampler2D u_baseColorTex;

void main() {
	vec4 albedo = u_baseColor;
	if (u_hasBaseColorTex) {
		albedo *= texture(u_baseColorTex, TexCoord);
	}

	FragColor = albedo;
}

