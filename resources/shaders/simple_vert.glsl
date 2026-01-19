#version 330 core

#define PI 3.1415926538

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

uniform sampler3D blackHoleMesh;
uniform vec3 blackHolePosition;
uniform float blackHoleSize;
uniform float blackHoleVertexMin;
uniform float blackHoleVertexMax;
uniform float blackHoleObserverMin;
uniform float blackHoleObserverMax;
uniform bool useBlackHole;
uniform bool blackHoleSecondary;
uniform bool freeCam;

uniform bool flipNormals;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

// max light count kinda low cause i'm lazy
const int MAX_DIR_LIGHTS = 3;
const int MAX_POINT_LIGHTS = 3;
const int MAX_TOTAL_LIGHTS = 6;

uniform vec3 dirLightDirections[MAX_DIR_LIGHTS];
uniform float dirLightIntensities[MAX_DIR_LIGHTS];

uniform vec3 pointLightPositions[MAX_POINT_LIGHTS];
uniform float pointLightIntensities[MAX_POINT_LIGHTS];

out vec3 meshPosition;
out vec3 scenePosition;
out vec3 viewPosition;
out vec3 postBHPosition;

out vec3 meshNormal;
out vec3 sceneNormal;
out vec3 viewNormal;
out vec3 postBHNormal;

out vec2 vTexCoord;

out vec3 viewBlackHolePosition;
out float blackHolePrimaryMinAngle;
out float blackHoleSecondaryMinAngle;
out vec3 viewBlackHoleObserver;

out vec3 lightDirections[MAX_TOTAL_LIGHTS];
out float lightIntensities[MAX_TOTAL_LIGHTS];

// https://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

float map(float value, float min1, float max1, float min2, float max2)
{
	return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void main()
{
	vec3 fixedCameraPosition = vec3(1.0, 2.0, 5.0);
	vec3 bhObserver;

	vec4 meshPositionV4 = vertPos;
	vec4 scenePositionV4 = M * meshPositionV4;
	vec4 viewPositionV4 = V * scenePositionV4;

	vec4 meshNormalV4 = flipNormals ? vec4(-1 * vertNor, 0.0) : vec4(vertNor, 0.0);
	vec4 sceneNormalV4 =  M * meshNormalV4;
	vec4 viewNormalV4 = V * sceneNormalV4;

	
	// for black hole stuff
	if (useBlackHole)
	{
		vec3 bhHole = (V * vec4(blackHolePosition, 1.0)).xyz;
		vec3 bhVertex = viewPositionV4.xyz;
		// camera will be at 0/0/0
		bhObserver = freeCam ? (V * vec4(fixedCameraPosition, 1.0)).xyz : vec3(0.0);
		viewBlackHoleObserver = bhObserver;

		vec3 bhXAxis = normalize(bhObserver - bhHole);
		vec3 bhNormal = normalize(cross(bhXAxis, bhVertex - bhHole));
		vec3 bhYAxis = normalize(cross(bhNormal, bhXAxis));

		vec3 bhVertexRelative = bhVertex - bhHole;
		float bhVertex2dX = dot(bhVertexRelative, bhXAxis);
		float bhVertex2dY = dot(bhVertexRelative, bhYAxis);
		vec2 bhVertex2dCart = vec2(bhVertex2dX, bhVertex2dY);
	
		vec3 bhObserverRelative = bhObserver - bhHole;
		float bhObserver2dX = dot(bhObserverRelative, bhXAxis);
		float bhObserver2dY = dot(bhObserverRelative, bhYAxis);
		vec2 bhObserver2dCart = vec2(bhObserver2dX, bhObserver2dY);

		float vertexR = length(bhVertex2dCart);
		float vertexPhi = atan(bhVertex2dY, bhVertex2dX);
		if (blackHoleSecondary)
		{
			vertexPhi = 2 * PI - vertexPhi;
		}
		float observerR = length(bhObserver2dCart);
	
		float vertexRMapped = map(vertexR / blackHoleSize, blackHoleVertexMin, blackHoleVertexMax, 0.0, 1.0);
		float vertexPhiMapped = map(vertexPhi, 0, 2 * PI, 0.0, 1.0);
		float observerRMapped = map(observerR / blackHoleSize, blackHoleObserverMin, blackHoleObserverMax, 0.0, 1.0);

		vec3 coord = vec3(observerRMapped, vertexPhiMapped, vertexRMapped);
		vec3 bh = texture(blackHoleMesh, coord).xyz;
		float va = bh.x;
		float oa = bh.y;
		float d = bh.z;
		
		// distances outside the range of the map need to be compensated
		d += max(0, observerR - blackHoleObserverMax * blackHoleSize);
		d += max(0, vertexR - blackHoleVertexMax * blackHoleSize);

		vec3 directionFromObserver = cos(oa) * bhXAxis + sin(oa) * (blackHoleSecondary ? -bhYAxis : bhYAxis);
		vec3 displacementFromObserver = directionFromObserver * d;
		vec3 normalRotationAxis = cross(bhXAxis, bhYAxis);
		float normalRotationAmount = (oa + PI) - va; // probably right?
		mat4 normalRotationMatrix = rotationMatrix(normalRotationAxis, normalRotationAmount);

		postBHPosition = bhObserver + displacementFromObserver;
		postBHNormal = (normalRotationMatrix * viewNormalV4).xyz;

		viewBlackHolePosition = bhHole;
		vec3 closestPrimaryRayToHorizon = vec3(observerRMapped, 0.5, vertexRMapped);
		blackHolePrimaryMinAngle = PI - texture(blackHoleMesh, closestPrimaryRayToHorizon).y;
		vec3 closestSecondaryRayToHorizon = vec3(observerRMapped, 1.0, vertexRMapped);
		blackHoleSecondaryMinAngle = PI - texture(blackHoleMesh, closestSecondaryRayToHorizon).y;
	}
	

	for (int i = 0; i < MAX_DIR_LIGHTS; i++)
	{
		lightDirections[i] = normalize((V * vec4(-dirLightDirections[i], 0.0)).xyz);
		lightIntensities[i] = dirLightIntensities[i];
	}
	for (int i = 0; i < MAX_POINT_LIGHTS; i++)
	{
		lightDirections[i + 3] = (V * vec4(pointLightPositions[i], 1.0) - V * M * vertPos).xyz;
		lightIntensities[i + 3] = pointLightIntensities[i];
	}

	
	meshPosition = meshPositionV4.xyz;
	scenePosition = scenePositionV4.xyz;
	if (freeCam)
	{
		viewPosition = scenePosition + fixedCameraPosition;
	}
	else
	{
		viewPosition = viewPositionV4.xyz;
	}
	
	meshNormal = normalize(meshNormalV4.xyz);
	sceneNormal = normalize(sceneNormalV4.xyz);
	viewNormal = normalize(viewNormalV4.xyz);

	if (useBlackHole)
	{
		gl_Position = P * vec4(postBHPosition, 1.0);
	}
	else
	{
		gl_Position = P * viewPositionV4;
	}

	vTexCoord = vertTex;
}
