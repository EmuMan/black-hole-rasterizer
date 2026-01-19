#version 330 core

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

out vec4 color;

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
			if (bhAngle < blackHoleSecondaryMinAngle || originalAngle < 0.05)
			{
				discard;
			}
		}
		else
		{
			if (bhAngle < blackHolePrimaryMinAngle - 0.03)
			{
				discard;
			}
		}
	}

	// Map normal in the range [-1, 1] to color in range [0, 1];
	vec3 nColor = 0.5 * viewNormal + 0.5;
	color = vec4(nColor, 1.0);
}
