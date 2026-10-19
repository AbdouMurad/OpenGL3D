#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in float Height;
in float Random;
in float Flatten; //0 standing .. 1 pressed flat by the trail
in vec4 FragPosLightSpace;

uniform vec3 u_cameraPos;
uniform vec3 u_rootColor;
uniform vec3 u_tipColor;

//same light uniforms as pbr.frag, so the renderer fills them the same way
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
struct DirectionalLight {
	vec3 direction;
	vec3 color;
	float intensity;
};
uniform int u_directionalLightCount;
uniform DirectionalLight u_directionalLights[MAX_DIRECTIONAL_LIGHTS];

uniform bool u_hasShadowMap;
uniform sampler2DArray u_shadowMap;

uniform bool u_grayWorld;
uniform vec3 u_fogColor;
uniform float u_fogDensity;

//same as pbr.frag
float colorReveal() {
	if (!u_grayWorld) return 1.0;
	float reveal = 0.0;
	for (int i = 0; i < u_lightCount; ++i) {
		float d = length(u_lights[i].position - FragPos);
		reveal = max(reveal, 1.0 - smoothstep(u_lights[i].range * 0.7, u_lights[i].range * 0.9, d));
	}
	return reveal;
}

//pbr.frag's shadow lookup, with a fixed bias (blades face every way, a slope bias would flicker)
float shadowFactor() {
	if (!u_hasShadowMap) return 0.0;
	vec3 p = FragPosLightSpace.xyz / FragPosLightSpace.w;
	p = p * 0.5 + 0.5;
	if (p.z > 1.0 || p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0) return 0.0;
	vec2 texel = 1.0 / vec2(textureSize(u_shadowMap, 0).xy);
	float shadow = 0.0;
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			float closest = texture(u_shadowMap, vec3(p.xy + vec2(x, y) * texel, 0.0)).r;
			shadow += (p.z - 0.003 > closest) ? 1.0 : 0.0;
		}
	}
	return shadow / 9.0;
}

void main() {
	//dark at the root, bright at the tip, every blade a slightly different shade, a few dried out ones
	vec3 albedo = mix(u_rootColor, u_tipColor, Height);
	albedo *= mix(0.7, 1.2, Random);
	albedo = mix(albedo, vec3(0.42, 0.36, 0.12), step(0.93, Random) * 0.7);
	//pressed grass shows its lighter, flat side: the trail reads as a paler path, like a trampled field
	albedo *= 1.0 + 0.35 * Flatten;

	float luminance = dot(albedo, vec3(0.2126, 0.7152, 0.0722));
	if (u_grayWorld) {
		//coloured areas get extra saturation (pushed past the original colour), gray areas are darkened
		//-> the lit circles stand out more against the gray
		float reveal = colorReveal();
		vec3 saturated = max(mix(vec3(luminance), albedo, 1.45), 0.0);
		albedo = mix(vec3(luminance) * 0.6, saturated, reveal);
	}

	vec3 N = normalize(Normal);
	vec3 lighting = vec3(0.0);

	//wrapped diffuse (light reaches a bit past 90 degrees): thin blades let light through, never go fully black
	for (int i = 0; i < u_lightCount; ++i) {
		vec3 toLight = u_lights[i].position - FragPos;
		float distance = length(toLight);
		float attenuation = clamp(1.0 - distance / u_lights[i].range, 0.0, 1.0);
		attenuation *= attenuation;
		float wrap = max(dot(N, toLight / distance) * 0.6 + 0.4, 0.0);
		lighting += u_lights[i].color * u_lights[i].intensity * attenuation * wrap;
	}
	for (int i = 0; i < u_directionalLightCount; ++i) {
		vec3 L = normalize(-u_directionalLights[i].direction);
		float wrap = max(dot(N, L) * 0.6 + 0.4, 0.0);
		float shadow = (i == 0) ? shadowFactor() : 0.0;
		lighting += (1.0 - shadow * 0.8) * u_directionalLights[i].color * u_directionalLights[i].intensity * wrap;
	}

	//roots sit in the shade of the blades around them
	float occlusion = mix(0.3, 1.0, Height);
	//diffuse only: /pi like the pbr shader's diffuse, so grass and objects match in brightness
	vec3 color = albedo / 3.14159 * lighting * occlusion + albedo * 0.03 * occlusion;

	if (u_fogDensity > 0.0) {
		float fogAmount = 1.0 - exp(-pow(length(u_cameraPos - FragPos) * u_fogDensity, 2.0));
		color = mix(color, u_fogColor, fogAmount);
	}
	FragColor = vec4(color, 1.0);
}
