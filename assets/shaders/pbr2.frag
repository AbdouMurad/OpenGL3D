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

float attenuation(float d, float r) {
	float r2 = r * r;
	float d2 = d*d;
	float num = max(1 - (d2 * d2) / (r2 * r2), 0);
	num = num * num;

	return num / (d2 + 0.0001);
}
float attenuation2(float distance, float r) {
    return 1.0 / (distance * distance);
}

vec3 getNormal() {
	vec3 N = normalize(Normal);
	if (!u_hasNormalMap) return N;

	vec3 T = normalize(Tangent);
	T = normalize(T - dot(T, N) * N);

	vec3 B = cross(N, T) * TangentSign;

	mat3 TBN = mat3(T,B,N);

	vec3 normalMap = texture(u_normalTex, TexCoord).rgb;

	normalMap = normalMap * 2.0 - 1.0;

	normalMap.xy *= u_normalScale;
	return normalize(TBN * normalMap);
}

//Fresnel-Schlick Function
vec3 F(vec3 F0, vec3 V, vec3 H) {
	return F0 + (1.0 - F0) * pow(1.0 - max(dot(V, H), 0.0), 5.0);	
}

//Lambertian
vec3 lambert(vec3 color) {
	return color / 3.14159265;
}


//Schlick-Beckmann Geometry Shadowing Function
float G1(float NdotX, float a) {
	float k = a / 2.0;
	float denom = NdotX * (1.0 - k) + k;
	denom = max(denom, 0.000001);

	return NdotX / denom;
}

// G Smith Model
float G(float alpha, float NdotV, float NdotL) {
	return G1(max(NdotV, 0.0), alpha) * G1(max(NdotL, 0.0), alpha);
}

//GGX/Trowbridge-Reits Normal Distribution Function
float D(float a2, float NdotH) {
	float NdotH2 = NdotH * NdotH;
	float denominator = max(3.14159265 * pow((NdotH2 * (a2 - 1.0) + 1), 2), 0.000001);
	return a2 / denominator;
}

void main() {
	//setup
	vec4 albedo = u_baseColor;
	if (u_hasBaseColorTex) {
		albedo *= texture(u_baseColorTex, TexCoord);
	}

	float metallic = u_metallic;
	float roughness = u_roughness;
	if (u_hasMetallicRoughnessTex) {
		vec4 mr = texture(u_metallicRoughnessTex, TexCoord);
		roughness *= mr.g;
		metallic *= mr.b;
	}
	roughness = clamp(roughness, 0.04, 1.0);

	vec3 N = getNormal();
	vec3 V = normalize(u_cameraPos - FragPos);
	
	float NdotV = max(dot(N, V), 0.0);

	vec3 F0 = vec3(0.04);
	F0 = mix(
		F0, 
		albedo.rgb,
		metallic
	);

	vec3 lighting = vec3(0.0);

	float a = roughness * roughness;
	
	//point lights
	for (int i = 0; i < u_lightCount; ++i) {
		vec3 L = normalize(u_lights[i].position - FragPos);
		vec3 H = normalize(V + L);
		float NdotL = max(dot(N, L), 0.0);

		//BRDF - can move to function
		vec3 ks = F(F0, V, H);
		vec3 kd = (1.0 - ks) * (1.0 - metallic);

		vec3 lam = lambert(albedo.rgb);

		float NdotH = max(dot(N, H), 0.0);
		
		vec3 cookTorranceNumerator = D(a * a, NdotH) * ks * G(a, NdotV, NdotL);
		
		float cookTorranceDenominator = max(4.0 * NdotV * NdotL, 0.0001);

		vec3 cookTorrance = cookTorranceNumerator / cookTorranceDenominator;

		vec3 BRDF = kd * lam + cookTorrance;

		lighting += BRDF * u_lights[i].color * NdotL * attenuation(distance(u_lights[i].position, FragPos), u_lights[i].range) * u_lights[i].intensity;
	}

	FragColor = vec4(lighting, albedo.a);
}