#include "Scene.h"
#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

BlackHoleMap::BlackHoleMap() :
	position(glm::vec3(0.0)),
	size(0.0f),
	vrResolution(0),
	vrMin(0.0f),
	vrMax(0.0f),
	vPhiResolution(0),
	orResolution(0),
	orMin(0.0f),
	orMax(0.0f),
	data(nullptr),
	textureID(0),
	textureUnit(0)
{
}

BlackHoleMap::~BlackHoleMap()
{
}

void BlackHoleMap::loadFromFile(std::string path)
{
	std::ifstream file(path);
	if (!file) {
		std::cerr << "Failed to open black hole file: " << path << std::endl;
		return;
	}

	// Read header: dimensions and min/max values.
	file >> vrResolution >> vPhiResolution >> orResolution;
	file >> vrMin >> vrMax;
	file >> orMin >> orMax;

	auto totalSize = vrResolution * vPhiResolution * orResolution * 3;

	// Allocate memory for observerAngles (3D array)
	data = new float[totalSize];

	// Read all the values into the observerAngles array
	for (int i = 0; i < totalSize; i += 3)
	{
		file >> data[i] >> data[i + 1] >> data[i + 2];
	}

	file.close();
}

void BlackHoleMap::sendToGPU()
{
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_3D, textureID);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, orResolution);
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, vPhiResolution);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, orResolution, vPhiResolution, vrResolution, 0, GL_RGB, GL_FLOAT, data);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_3D, 0);
}

glm::vec3 BlackHoleMap::getValue(float vr, float vPhi, float orAngle)
{
	if (vr >= 1.0f)
		vr = 0.9999f;
	if (vr < 0.0f)
		vr = 0.0f;

	if (vPhi >= 1.0f)
		vPhi = 0.9999f;
	if (vPhi < 0.0f)
		vPhi = 0.0f;

	if (orAngle >= 1.0f)
		orAngle = 0.9999f;
	if (orAngle < 0.0f)
		orAngle = 0.0f;

	int vrIndex = floorf(vr * vrResolution);
	int vPhiIndex = floorf(vPhi * vPhiResolution);
	int orIndex = floorf(orAngle * orResolution);

	std::cout << "vrIndex: " << vrIndex << ", vPhiIndex: " << vPhiIndex << ", orIndex: " << orIndex << std::endl;

	int baseIndex = (vrIndex * vPhiResolution * orResolution + vPhiIndex * orResolution + orIndex) * 3;

	return glm::vec3(data[baseIndex], data[baseIndex + 1], data[baseIndex + 2]);
}

void BlackHoleMap::bind(GLint handle)
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_3D, textureID);
	glUniform1i(handle, textureUnit);
}

void BlackHoleMap::unbind()
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_3D, 0);
}

Scene::Scene() :
	nextAvailableId(0),
	currentShaderProgramIndex(0),
	shaderPrograms(std::vector<std::shared_ptr<Program>>()),
	objects(std::vector<std::shared_ptr<Object>>()),
	blackHole(nullptr),
	activeCamera(nullptr),
	viewMatrix(glm::identity<glm::mat4>()),
	projectionMatrix(glm::identity<glm::mat4>())
{
}

Scene::~Scene()
{
}

long Scene::getNextAvailableId()
{
	return nextAvailableId++;
}

void Scene::computeCameraMatrices()
{
	if (activeCamera == nullptr)
	{
		return;
	}

	glm::vec3 eye = activeCamera->getGlobalPosition();
	glm::vec3 target = eye + activeCamera->getFacing();
	glm::vec3 up = glm::vec3(0, 1, 0);
	viewMatrix = glm::lookAt(eye, target, up);
	projectionMatrix = activeCamera->getProjectionMatrix();
}

void Scene::drawAll(bool freeCam)
{
	computeCameraMatrices();
	for (auto& object : objects)
	{
		object->draw(freeCam);
	}
}

void Scene::addObject(std::shared_ptr<Object> newObject)
{
	objects.push_back(newObject);
}

void Scene::addShaderProgram(std::shared_ptr<Program> newShaderProgram)
{
	shaderPrograms.push_back(newShaderProgram);
}

void Scene::swapToShaderProgram(int shaderProgramIndex)
{
	if (shaderProgramIndex != currentShaderProgramIndex)
	{
		currentShaderProgramIndex = shaderProgramIndex;
		shaderPrograms[currentShaderProgramIndex]->bind();
	}
}

std::shared_ptr<Program> Scene::getCurrentShaderProgram()
{
	return shaderPrograms[currentShaderProgramIndex];
}

void Scene::addLightsToProgram(std::shared_ptr<Program> program)
{
	int dirLightIndex = 0;
	glm::vec3 dirLightDirections[MAX_DIR_LIGHTS] = { glm::vec3(0.0) };
	float dirLightIntensities[MAX_DIR_LIGHTS] = { 0.0 };
	int pointLightIndex = 0;
	glm::vec3 pointLightPositions[MAX_POINT_LIGHTS] = { glm::vec3(0.0) };
	float pointLightIntensities[MAX_POINT_LIGHTS] = { 0.0 };

	// TODO: make more robust (should be easy) (surely)
	dirLightDirections[0] = glm::vec3(1.0, -2.0, -1.0);
	dirLightIntensities[0] = 0.3;

	for (auto& object : objects)
	{
		// apparently this is bad but i don't really care
		if (PointLightObject* plo = dynamic_cast<PointLightObject*>(object.get()))
		{
			auto globalPos = plo->getGlobalPosition();
			pointLightPositions[pointLightIndex] = globalPos;
			pointLightIntensities[pointLightIndex] = plo->intensity;
			pointLightIndex++;
		}
	}

	glUniform3fv(program->getUniform("dirLightDirections"), MAX_DIR_LIGHTS, glm::value_ptr(dirLightDirections[0]));
	glUniform1fv(program->getUniform("dirLightIntensities"), MAX_DIR_LIGHTS, dirLightIntensities);
	glUniform3fv(program->getUniform("pointLightPositions"), MAX_POINT_LIGHTS, glm::value_ptr(pointLightPositions[0]));
	glUniform1fv(program->getUniform("pointLightIntensities"), MAX_POINT_LIGHTS, pointLightIntensities);
}

void Scene::addBlackHoleToProgram(std::shared_ptr<Program> program)
{
	if (blackHole == nullptr)
	{
		return;
	}
	glUniform3f(program->getUniform("blackHolePosition"), blackHole->position.x, blackHole->position.y, blackHole->position.z);
	glUniform1f(program->getUniform("blackHoleSize"), blackHole->size);
	glUniform1f(program->getUniform("blackHoleVertexMin"), blackHole->vrMin);
	glUniform1f(program->getUniform("blackHoleVertexMax"), blackHole->vrMax);
	glUniform1f(program->getUniform("blackHoleObserverMin"), blackHole->orMin);
	glUniform1f(program->getUniform("blackHoleObserverMax"), blackHole->orMax);
}

void Scene::evaluateAllGlobalTransforms()
{
	for (auto& object : objects)
	{
		if (object->parent == nullptr)
		{
			object->evaluateGlobalTransformsRecursive();
		}
	}
}
