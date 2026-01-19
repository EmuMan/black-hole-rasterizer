#version 330 core 

out vec4 color;

uniform vec3 matAmb;
uniform vec3 matDif;
uniform vec3 matSpec;
uniform float specIntensity;

const int MAX_TOTAL_LIGHTS = 6;

in vec3 meshPosition;
in vec3 scenePosition;
in vec3 viewPosition;
in vec3 postBHPosition;

in vec3 meshNormal;
in vec3 sceneNormal;
in vec3 viewNormal;
in vec3 postBHNormal;

uniform bool useBlackHole;
uniform bool blackHoleSecondary;
in vec3 viewBlackHolePosition;
in float blackHolePrimaryMinAngle;
in float blackHoleSecondaryMinAngle;
in vec3 viewBlackHoleObserver;

in vec3 lightDirections[MAX_TOTAL_LIGHTS];
in float lightIntensities[MAX_TOTAL_LIGHTS];

void main()
{
	float blackHoleDistSq = dot(viewBlackHolePosition, viewBlackHolePosition);
	float bhFragDistSq = dot(postBHPosition, postBHPosition);
	// TODO: really hacky fix?? why are distances wrong???
	if (useBlackHole && bhFragDistSq * 0.2 > blackHoleDistSq)
	{
		vec3 dirToVertex = normalize(postBHPosition);
		vec3 dirToOriginalVertex = normalize(viewPosition);
		vec3 dirToBlackHole = normalize(viewBlackHolePosition);
		float bhAngle = acos(clamp(dot(dirToVertex, dirToBlackHole), -1.0, 1.0));
		float originalAngle = acos(clamp(dot(dirToOriginalVertex, dirToBlackHole), -1.0, 1.0));
		if (blackHoleSecondary)
		{
			// TODO: unify with other one :(
			if (bhAngle < blackHoleSecondaryMinAngle)
			{
				discard;
			}
		}
		else
		{
			if (bhAngle < blackHolePrimaryMinAngle * 0.97)
			{
				discard;
			}
		}
	}

	//you will need to work with these for lighting
	vec3 finalColor = vec3(0.0);

	#pragma optionNV(unroll all)
	for (int i = 0; i < MAX_TOTAL_LIGHTS; i++)
	{
		vec3 lightDir = lightDirections[i];
		vec3 lightDirNorm = normalize(lightDir);
		float lightFalloff = 1.0 / max(dot(lightDir, lightDir), 1.0);
		float lightIntensity = lightIntensities[i] * lightFalloff;
		float dC = max(dot(viewNormal, lightDirNorm), 0.0);
		vec3 H = normalize((-normalize(viewPosition) + lightDirNorm) / 2);
		float sC = pow(max(dot(viewNormal, H), 0), specIntensity);
		finalColor += (matDif * dC + matSpec * sC) * lightIntensity;
	}

	color = vec4(matAmb + finalColor, 1.0);
}
