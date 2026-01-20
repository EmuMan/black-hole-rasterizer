#pragma once
#ifndef _SCENE_H_
#define _SCENE_H_

#include <vector>

class Object;
class CameraObject;
#include "Object.h"
#include "Program.h"
#include "MatrixStack.h"

constexpr auto MAX_TOTAL_LIGHTS = 6;
constexpr auto MAX_DIR_LIGHTS = 3;
constexpr auto MAX_POINT_LIGHTS = 3;

class BlackHoleMap {
public:
	BlackHoleMap();
	virtual ~BlackHoleMap();
	glm::vec3 position;
	float size;
	int vrResolution;
	float vrMax, vrMin;
	int vPhiResolution;
	int orResolution;
	float orMin, orMax;
	float* data;
	GLuint textureID;
	GLint textureUnit;
	void loadFromFile(std::string path);
	void sendToGPU();
	glm::vec3 getValue(float vr, float vPhi, float orAngle);
	void bind(GLint handle);
	void unbind();
};

class Scene {
private:
	long nextAvailableId;
public:
	Scene();
	virtual ~Scene();
	int currentShaderProgramIndex;
	std::vector<std::shared_ptr<Program>> shaderPrograms;
	std::vector<std::shared_ptr<Object>> objects;
	std::shared_ptr<BlackHoleMap> blackHole;
	std::shared_ptr<CameraObject> activeCamera;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	long getNextAvailableId();
	void computeCameraMatrices();
	void drawAll(bool freeCam);
	void addObject(std::shared_ptr<Object> newObject);
	void addShaderProgram(std::shared_ptr<Program> newShaderProgram);
	void swapToShaderProgram(int shaderProgramIndex);
	std::shared_ptr<Program> getCurrentShaderProgram();
	void addLightsToProgram(std::shared_ptr<Program> program);
	void addBlackHoleToProgram(std::shared_ptr<Program> program);
	void evaluateAllGlobalTransforms();
};

#endif
