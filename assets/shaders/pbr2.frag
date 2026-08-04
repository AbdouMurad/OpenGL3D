#version 330 core

vec3 fresnelSchlick(vec3 V, vec3 H, vec3 F0) {
	return F0 + (1.0 - F0) * pow(1.0 - max(dot(V, H), 0.0), 5.0);	
}

float distributionGGXTrowbridgeReitz(float NdotH, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;

	return a2 / (3.14159265359 * denom * denom);
}

float geometrySchlickBeckmann(float NdotV, float roughness) {
	float k = (roughness * roughness) / 2.0;
	return NdotV / (NdotV * (1.0 - k) + k);
}


//specular BRDF using Cook-Torrance model
float cookTorranceBRDF(vec3 N, vec3 V, vec3 L, float roughness, vec3 F0) {
	float NdotV = max(dot(N, V), 0.0);
	float LdotN = max(dot(L, N), 0.0);
	float DGF = 
		distributionGGXTrowbridgeReitz(max(dot(N, normalize(L + V)), 0.0), roughness)) *
		geometrySchlickBeckmann(NdotV, roughness) *
		fresnelSchlick(V, normalize(L + V), F0);
	
	return DGF / max(4.0 * NdotV * LdotN, 0.001);
}

// Lambertian diffuse BRDF
float orenNayarRealisticBRDF() {

}

//X = fragment position
//L = light Vector
//V = view vector
float BRDF(vec3 X, vec3 L, vec3 V) {

}