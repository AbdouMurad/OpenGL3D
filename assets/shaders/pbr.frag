#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;

in vec3 Normal;
in vec3 Tangent;
in float TangentSign;

uniform vec3 u_cameraPos;

uniform vec4 u_baseColor;

uniform float u_metallic;
uniform float u_roughness;

uniform bool u_hasMetallicRoughnessTex;
uniform sampler2D u_metallicRoughnessTex;

uniform bool u_hasBaseColorTex;
uniform sampler2D u_baseColorTex;

uniform bool u_hasNormalMap;
uniform sampler2D u_normalTex;
uniform float u_normalScale;

const int MAX_LIGHTS = 16;

struct PointLight {
	vec3 position;
	vec3 color;
	float intensity;
	float range;
};

uniform int u_lightCount;
uniform PointLight u_lights[MAX_LIGHTS];

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
float DistributionGGX(vec3 N, vec3 H, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;

	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;

	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = 3.14159 * denom * denom;

	return num /denom;
}
float GeometrySchlickGGX(float NdotV, float roughness) {
	float r = roughness + 1.0;
	float k = (r*r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num/denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
	float NdotV = max(dot(N,V), 0.0);
	float NdotL = max(dot(N,L), 0.0);

	float ggx1 = GeometrySchlickGGX(NdotV, roughness);
	float ggx2 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 getNormal() {
	vec3 N = normalize(Normal);

	if (!u_hasNormalMap) return N;

	vec3 T = normalize(Tangent);

	T = normalize(T - dot(T, N) * N);

	vec3 B = cross(N, T) * TangentSign;

	mat3 TBN = mat3(T, B, N);

	vec3 normalMap = texture(u_normalTex, TexCoord).rgb;

	normalMap = normalMap * 2.0 - 1.0;

	normalMap.xy *= u_normalScale;

	return normalize(TBN * normalMap);
}


void main() {
	vec4 albedo = u_baseColor;
	if (u_hasBaseColorTex) {
		albedo *= texture(u_baseColorTex, TexCoord);
	}

	float metallic = u_metallic;
	float roughness = u_roughness;

	if (u_hasMetallicRoughnessTex) {
		vec4 mr = texture(
			u_metallicRoughnessTex,
			TexCoord
		);
		roughness *= mr.g;
		metallic *= mr.b;
	}
	roughness = clamp(roughness, 0.04, 1.0);

	vec3 viewDir = normalize(u_cameraPos - FragPos);

	vec3 norm = getNormal();

	vec3 lighting = vec3(0.0);

	vec3 N = norm;
	vec3 V = viewDir;


	for (int i = 0; i < u_lightCount; ++i)
	{
		vec3 L = normalize(
			u_lights[i].position - FragPos
		);

		vec3 H = normalize(V + L);

		float distance = length(u_lights[i].position - FragPos);

		float attenuation = clamp(
			1.0 - distance / u_lights[i].range,
			0.0,
			1.0
		);

		attenuation *= attenuation;


		vec3 radiance = u_lights[i].color * u_lights[i].intensity * attenuation;

		// Reflectance at normal incidence
		vec3 F0 = vec3(0.04);

		// Metals use their albedo as F0
		F0 = mix(
			F0,
			albedo.rgb,
			metallic
		);

		float HdotV = clamp(dot(H, V), 0.0, 1.0);
		vec3 F = fresnelSchlick(HdotV, F0);

		float NDF = DistributionGGX(
					N,
					H,
					roughness
				);

		float G = GeometrySmith(
					N,
					V,
					L,
					roughness
				);


		vec3 numerator = NDF * G * F;

		float denominator =
			4.0 *
			max(dot(N,V),0.0) *
			max(dot(N,L),0.0)
			+ 0.001;

		vec3 specular = numerator / denominator;

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;

		// Metals don't have diffuse reflection
		kD *= 1.0 - metallic;


		float NdotL =
			max(dot(N,L),0.0);


		vec3 diffuse =
			kD *
			albedo.rgb /
			3.14159;


		lighting +=
			(diffuse + specular)
			*
			radiance *
			NdotL;
	}

	vec3 ambient = vec3(0.03) * albedo.rgb * (1.0 - metallic);
	lighting += ambient;

	vec3 finalColor = lighting;
	FragColor = vec4(finalColor, albedo.a);
}

