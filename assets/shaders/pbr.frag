#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform vec4 u_baseColor;

uniform bool u_hasBaseColorTex;
uniform sampler2D u_baseColorTex;

const int MAX_LIGHTS = 16;

struct PointLight {
	vec3 position;
	vec3 color;
	float intensity;
};

uniform int u_lightCount;
uniform PointLight u_lights[MAX_LIGHTS];

void main() {
	vec4 albedo = u_baseColor;
	if (u_hasBaseColorTex) {
		albedo *= texture(u_baseColorTex, TexCoord);
	}

	vec3 norm = normalize(Normal);

	vec3 lighting = vec3(0.0);

	for (int i = 0; i < u_lightCount; ++i) {
		vec3 lightDir = normalize(u_lights[i].position - FragPos);
		
		float diffuse = max(dot(norm, lightDir), 0.0);

		float distance = length(u_lights[i].position - FragPos);

		float attenuation = 1.0 / (1.0 + 0.01 * distance);

		lighting += diffuse * u_lights[i].color * u_lights[i].intensity * attenuation;
	}

	lighting += vec3(0.1);
	vec3 finalColor = albedo.rgb * lighting;

	FragColor = vec4(finalColor, albedo.a);
}

