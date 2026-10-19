#version 330 core

//blade mesh: a flat strip 1 tall, 1 wide at the root (see GrassField::BuildBlade)
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord; //y = 0 at the root .. 1 at the tip

//per blade (instanced)
layout (location = 4) in vec4 iRootAndYaw; //xyz = root, w = rotation around y
layout (location = 5) in vec4 iParams;	   //x = height, y = width, z = lean, w = random 0..1

out vec3 FragPos;
out vec3 Normal;
out float Height;	//0 root .. 1 tip
out float Random;
out vec4 FragPosLightSpace;

uniform mat4 camMatrix;
uniform mat4 u_lightMatrix;

uniform float u_time;
uniform vec2 u_windDirection; //normalized, on the ground (x, z)
uniform float u_windStrength;

//something walking through the grass pushes it aside (radius 0 = nothing)
uniform vec3 u_pusherPosition;
uniform float u_pusherRadius;

//trail map (see GrassField::Stamp): rg = direction pressed, b = when, a = how hard
uniform sampler2D u_trailMap;
uniform vec4 u_trailRect;		 //xy = world x/z of the map's corner, zw = 1 / its size
uniform float u_trailRecoverTime; //seconds to stand back up

out float Flatten; //0 standing .. 1 pressed flat

void main() {
	vec3 root = iRootAndYaw.xyz;
	float yaw = iRootAndYaw.w;
	float height = iParams.x;
	float width = iParams.y;
	float lean = iParams.z;
	float random = iParams.w;
	float h = aTexCoord.y;

	//turn the strip around y. side = across the blade, facing = the way its flat side points
	vec3 side = vec3(cos(yaw), 0.0, -sin(yaw));
	vec3 facing = vec3(sin(yaw), 0.0, cos(yaw));

	//---- trail: how flat this blade is still pressed, and which way ----
	vec4 trail = textureLod(u_trailMap, (root.xz - u_trailRect.xy) * u_trailRect.zw, 0.0);
	float age = u_time - trail.b;
	//same curve as GrassField::Remaining: stays down for the first 35% of the time, then eases back up
	float flatten = trail.a * (1.0 - smoothstep(0.35, 1.0, clamp(age / u_trailRecoverTime, 0.0, 1.0)));
	//each blade lies a little off the path direction, so the trail doesn't look combed
	float spread = (random - 0.5) * 0.7;
	vec2 layDirection = mat2(cos(spread), sin(spread), -sin(spread), cos(spread)) * trail.rg;

	//---- bending: sideways offset that grows towards the tip, in blade heights ----
	//resting lean, each blade its own way
	vec2 leanDirection = vec2(cos(random * 40.0), sin(random * 40.0));
	vec2 bend = leanDirection * lean * (1.0 - flatten);

	//wind: waves rolling across the field along the wind direction
	//0.5 + 0.5 * sin -> 0..1: each blade bends over with the wave and comes back to upright, then repeats
	//a smaller, faster flutter rides on top so neighbouring blades don't move in lockstep
	//pressed blades are pinned down, they barely move in the wind
	float wave = dot(root.xz, u_windDirection) * 0.22 - u_time * 1.6;
	float gust = 0.5 + 0.5 * sin(wave);
	float flutter = 0.12 * sin(u_time * 4.3 + random * 6.283);
	bend += u_windDirection * (gust * 0.5 + flutter) * u_windStrength * (1.0 - 0.85 * flatten);

	//laid down along the path
	bend += layDirection * flatten * 0.95;

	//pushed away from whoever is standing in it right now (only when they're near the ground)
	if (u_pusherRadius > 0.0) {
		vec2 away = root.xz - u_pusherPosition.xz;
		float d = length(away);
		float closeness = 1.0 - smoothstep(u_pusherRadius * 0.4, u_pusherRadius, d);
		float low = 1.0 - smoothstep(1.0, 2.0, u_pusherPosition.y - root.y);
		if (d > 0.0001) bend += away / d * closeness * low * 0.9;
	}

	//a blade can tip over at most to nearly flat. the tip swings sideways by `amount` and drops so the
	//blade keeps its length (sqrt(1 - amount^2)), instead of stretching sideways
	float amount = min(length(bend), 0.95);
	if (amount > 0.0) bend *= amount / length(bend);
	//h^2: the root stays put, the tip moves most -> a curve rather than a tilted stick
	float curve = h * h;
	vec3 position = root + side * aPos.x * width;
	position.xz += bend * curve * height;
	position.y += h * height * sqrt(1.0 - amount * amount * curve);

	//halfway between the blade's side and straight up: grass lit like a soft carpet, not flat cards
	//flattened blades face the sky
	Normal = normalize(mix(facing, vec3(0.0, 1.0, 0.0), mix(0.6, 0.9, flatten)));
	Height = h;
	Random = random;
	Flatten = flatten;
	FragPos = position;
	FragPosLightSpace = u_lightMatrix * vec4(position, 1.0);
	gl_Position = camMatrix * vec4(position, 1.0);
}
