#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;

in vec3 Normal;
in vec3 Tangent;
in float TangentSign;
in vec4 FragPosLightSpace;

uniform vec3 u_cameraPos;

uniform vec4 u_baseColor;

uniform float u_metallic;
uniform float u_roughness;

uniform bool u_hasMetallicRoughnessTex;
uniform sampler2D u_metallicRoughnessTex;

uniform bool u_hasBaseColorTex;
uniform sampler2D u_baseColorTex;

uniform bool u_hasNormalTex;
uniform sampler2D u_normalTex;
uniform float u_normalScale;

uniform vec3 u_emissiveFactor;
uniform bool u_hasEmissiveTex;
uniform sampler2D u_emissiveTex;

const int MAX_LIGHTS = 16;

struct PointLight {
	vec3 position;
	vec3 color;
	float intensity;
	float range;
};

uniform int u_lightCount;
uniform PointLight u_lights[MAX_LIGHTS];

const int MAX_DIRECTIONAL_LIGHTS = 4;

//sun-like: same direction everywhere, no falloff
struct DirectionalLight {
	vec3 direction; //the way the light travels (world space, normalized)
	vec3 color;
	float intensity;
};

uniform int u_directionalLightCount;
uniform DirectionalLight u_directionalLights[MAX_DIRECTIONAL_LIGHTS];

//shadow map of directional light 0 (rendered by the shadow pass from the light's point of view)
uniform bool u_hasShadowMap;
uniform sampler2DArray u_shadowMap;

//0 = fully lit, 1 = fully in shadow. N = surface normal, L = direction towards the light
float shadowFactor(vec3 N, vec3 L) {
	if (!u_hasShadowMap) return 0.0;

	//light clip space -> 0..1: xy = where to read the shadow map, z = this point's depth from the light
	vec3 p = FragPosLightSpace.xyz / FragPosLightSpace.w;
	p = p * 0.5 + 0.5;
	if (p.z > 1.0) return 0.0; //beyond the light box's far plane -> nothing recorded, treat as lit

	//bias: compare against "a bit closer to the light" so limited depth precision doesn't shadow a surface with itself (acne)
	//surfaces at a grazing angle to the light need more (one shadow map texel covers a longer slope)
	float bias = max(0.002 * (1.0 - dot(N, L)), 0.0005);

	//PCF: 3x3 neighbouring texels each give lit/shadowed, the average softens the jagged texel edges
	vec2 texel = 1.0 / vec2(textureSize(u_shadowMap, 0).xy);
	float shadow = 0.0;
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			float closest = texture(u_shadowMap, vec3(p.xy + vec2(x, y) * texel, 0.0)).r; //nearest surface the light sees there
			shadow += (p.z - bias > closest) ? 1.0 : 0.0; //something closer to the light -> this point is behind it
		}
	}
	return shadow / 9.0;
}

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

//light reaching the surface from direction L (towards the light) with the given radiance
//-> light reflected towards the viewer V. same Cook-Torrance GGX math for every light type
vec3 shadeLight(vec3 N, vec3 V, vec3 L, vec3 radiance, vec3 albedo, float metallic, float roughness) {
	vec3 H = normalize(V + L);

	// Reflectance at normal incidence, metals use their albedo as F0
	vec3 F0 = mix(vec3(0.04), albedo, metallic);

	float HdotV = clamp(dot(H, V), 0.0, 1.0);
	vec3 F = fresnelSchlick(HdotV, F0);

	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
	vec3 specular = numerator / denominator;

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	// Metals don't have diffuse reflection
	kD *= 1.0 - metallic;

	float NdotL = max(dot(N, L), 0.0);
	vec3 diffuse = kD * albedo / 3.14159;

	return (diffuse + specular) * radiance * NdotL;
}

vec3 getNormal() {
	vec3 N = normalize(Normal);

	if (!u_hasNormalTex) return N;

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
		vec3 L = normalize(u_lights[i].position - FragPos);

		float distance = length(u_lights[i].position - FragPos);
		float attenuation = clamp(1.0 - distance / u_lights[i].range, 0.0, 1.0);
		attenuation *= attenuation;

		vec3 radiance = u_lights[i].color * u_lights[i].intensity * attenuation;
		lighting += shadeLight(N, V, L, radiance, albedo.rgb, metallic, roughness);
	}

	for (int i = 0; i < u_directionalLightCount; ++i)
	{
		//towards the light = against the way it travels. no distance falloff
		vec3 L = normalize(-u_directionalLights[i].direction);
		vec3 radiance = u_directionalLights[i].color * u_directionalLights[i].intensity;
		//only light 0 has a shadow map. geometric normal for the bias (normal maps would make it noisy)
		float shadow = (i == 0) ? shadowFactor(normalize(Normal), L) : 0.0;
		lighting += (1.0 - shadow) * shadeLight(N, V, L, radiance, albedo.rgb, metallic, roughness);
	}

	vec3 ambient = vec3(0.03) * albedo.rgb * (1.0 - metallic);
	
	//glTF: emissive = factor * texture (factor defaults to black)
	vec3 emissive = u_emissiveFactor;
	if (u_hasEmissiveTex) {
		emissive *= texture(u_emissiveTex, TexCoord).rgb;
	}

	
	vec3 finalColor = lighting + ambient + emissive;
	FragColor = vec4(finalColor, albedo.a);
}

