#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 aTangent;

out vec2 TexCoord;
out vec3 FragPos;

out vec3 Normal;
out vec3 Tangent;
out float TangentSign;

uniform mat4 camMatrix;
uniform mat4 model;

void main() {
	vec3 N = normalize(mat3(model) * aNormal);
	vec3 T = normalize(mat3(model) * aTangent.xyz);

	T = normalize(T - dot(T,N) * N);

	TexCoord = aTexCoord;
	FragPos = vec3(model * vec4(aPos,1.0));

	Normal = N;
	Tangent = T;
	TangentSign = aTangent.w;

	gl_Position = camMatrix * vec4(FragPos, 1.0);
}