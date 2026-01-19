#pragma once
#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <vector>
#include <memory>

class Material;
#include "Model.h"
#include "Material.h"
#include "Scene.h"
#include "MatrixStack.h"

class Object
{
public:
	Object(std::shared_ptr<Scene> scene);
	virtual ~Object();
	std::shared_ptr<Scene> scene;
	long id;
	glm::vec3 translation;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::mat4 globalTransform;
	std::vector<std::shared_ptr<Object>> children;
	std::shared_ptr<Object> parent;
	// for now, these both need to be called.
	// not sure how to fix it with shared_ptr...
	void addChild(std::shared_ptr<Object> newChild);
	void setParent(std::shared_ptr<Object> newParent);
	void evaluateGlobalTransformsRecursive();
	void evaluateGlobalTransformsRecursive(std::shared_ptr<MatrixStack> M);
	glm::vec3 getGlobalPosition();
	virtual void draw(bool freeCam);
};

class MeshObject : public Object
{
public:
	MeshObject(std::shared_ptr<Scene> scene, std::shared_ptr<Model> model, std::shared_ptr<Material> material);
	std::shared_ptr<Model> model;
	std::shared_ptr<Material> material;
	void draw(bool freeCam) override;
};

class PointLightObject : public Object
{
public:
	PointLightObject(std::shared_ptr<Scene> scene, float intensity);
	float intensity;
};

class CameraObject : public Object
{
public:
	CameraObject(std::shared_ptr<Scene> scene, float fovy, float aspect, float zNear, float zFar);
	glm::vec3 getFacing();
	glm::vec3 getStrafe(glm::vec3 up);
	glm::mat4 getProjectionMatrix();
	float fovy, aspect, zNear, zFar;
};

#endif
