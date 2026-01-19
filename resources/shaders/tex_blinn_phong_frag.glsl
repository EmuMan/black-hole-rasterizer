#version 330 core 

out vec4 color;

uniform sampler2D Texture0;
uniform float amb;
uniform float dif;
uniform float spec;
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

in vec2 vTexCoord;

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
	vec3 blackHoleDisp = viewBlackHolePosition - viewBlackHoleObserver;
	float blackHoleDistSq = dot(blackHoleDisp, blackHoleDisp);
	vec3 bhFragDisp = postBHPosition - viewBlackHoleObserver;
	float bhFragDistSq = dot(bhFragDisp, bhFragDisp);
	// TODO: really hacky fix?? why are distances wrong???
	if (useBlackHole && bhFragDistSq * 0.2 > blackHoleDistSq)
	{
		vec3 dirToVertex = normalize(bhFragDisp);
		vec3 dirToOriginalVertex = normalize(viewPosition - viewBlackHoleObserver);
		vec3 dirToBlackHole = normalize(viewBlackHolePosition - viewBlackHoleObserver);
		float bhAngle = acos(clamp(dot(dirToVertex, dirToBlackHole), -1.0, 1.0));
		float originalAngle = acos(clamp(dot(dirToOriginalVertex, dirToBlackHole), -1.0, 1.0));
		if (blackHoleSecondary)
		{
			if (bhAngle < blackHoleSecondaryMinAngle || originalAngle < 0.04)
			{
				discard;
			}
		}
		else
		{
			if (bhAngle < blackHolePrimaryMinAngle * 0.91)
			{
				discard;
			}
		}
	}

	//you will need to work with these for lighting
	vec3 normal = normalize(viewNormal);
	vec3 texColor0 = texture(Texture0, vTexCoord).xyz;
	vec3 finalColor = vec3(0.0);

	#pragma optionNV(unroll all)
	for (int i = 0; i < MAX_TOTAL_LIGHTS; i++)
	{
		vec3 lightDir = lightDirections[i];
		vec3 lightDirNorm = normalize(lightDir);
		float lightFalloff = 1.0 / max(dot(lightDir, lightDir), 1.0);
		float lightIntensity = lightIntensities[i] * lightFalloff;
		float dC = max(dot(normal, lightDirNorm), 0.0);
		vec3 H = normalize((-normalize(viewPosition) + lightDirNorm) / 2);
		float sC = pow(max(dot(normal, H), 0), specIntensity);
		finalColor += (texColor0 * dC * dif + texColor0 * sC * spec) * lightIntensity;
	}

	color = vec4(texColor0 * amb + finalColor, 1.0);
}
