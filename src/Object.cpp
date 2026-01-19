#include "Object.h"
#include "Program.h"
#include "glad/glad.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Object::Object(std::shared_ptr<Scene> scene) :
	scene(scene),
	translation(glm::vec3()),
	rotation(glm::vec3()),
	scale(glm::vec3(1)),
	globalTransform(glm::mat4(1.0)),
	children(std::vector<std::shared_ptr<Object>>()),
	parent(nullptr)
{
	id = scene->getNextAvailableId();
}

Object::~Object()
{
}

void Object::setParent(std::shared_ptr<Object> newParent)
{
	if (parent != nullptr)
	{
		int lastIndex = -1;
		for (auto i = 0; i < parent->children.size(); i++)
		{
			if (parent->children[i]->id == id)
			{
				lastIndex = id;
				break;
			}
		}
		if (lastIndex != -1)
		{
			parent->children.erase(parent->children.begin() + lastIndex);
		}
	}

	parent = newParent;
}

void Object::addChild(std::shared_ptr<Object> newChild)
{
	children.push_back(newChild);
}

void Object::evaluateGlobalTransformsRecursive()
{
	evaluateGlobalTransformsRecursive(std::make_shared<MatrixStack>());
}

void Object::evaluateGlobalTransformsRecursive(std::shared_ptr<MatrixStack> M)
{
	M->pushMatrix();
	M->translate(translation);
	M->rotate(rotation.y, glm::vec3(0, 1, 0));
	M->rotate(rotation.z, glm::vec3(0, 0, 1));
	M->rotate(rotation.x, glm::vec3(1, 0, 0));
	M->scale(scale);
	globalTransform = M->topMatrix();
	for (auto& child : children)
	{
		child->evaluateGlobalTransformsRecursive(M);
	}
	M->popMatrix();
}

glm::vec3 Object::getGlobalPosition()
{
	return globalTransform * glm::vec4(glm::vec3(0.0), 1.0);
}

void Object::draw(bool freeCam)
{
}

MeshObject::MeshObject(std::shared_ptr<Scene> scene, std::shared_ptr<Model> model, std::shared_ptr<Material> material) :
	Object(scene),
	model(model),
	material(material)
{
}

void MeshObject::draw(bool freeCam)
{
	material->apply(scene);
	auto& program = scene->getCurrentShaderProgram();
	scene->addLightsToProgram(program);
	scene->addBlackHoleToProgram(program);
	glUniformMatrix4fv(program->getUniform("M"), 1, GL_FALSE, glm::value_ptr(globalTransform));
	glUniformMatrix4fv(program->getUniform("V"), 1, GL_FALSE, glm::value_ptr(scene->viewMatrix));
	glUniformMatrix4fv(program->getUniform("P"), 1, GL_FALSE, glm::value_ptr(scene->projectionMatrix));
	glUniform1i(program->getUniform("freeCam"), freeCam);
	model->draw(program);
}

PointLightObject::PointLightObject(std::shared_ptr<Scene> scene, float intensity) :
	Object(scene),
	intensity(intensity)
{
}

CameraObject::CameraObject(std::shared_ptr<Scene> scene, float fovy, float aspect, float zNear, float zFar) :
	Object(scene),
	fovy(fovy),
	aspect(aspect),
	zNear(zNear),
	zFar(zFar)
{
}

glm::vec3 CameraObject::getFacing()
{
	MatrixStack stack;
	stack.pushMatrix();
	stack.loadIdentity();
	stack.rotate(rotation.y, glm::vec3(0, 1, 0));
	stack.rotate(rotation.z, glm::vec3(0, 0, 1));
	stack.rotate(rotation.x, glm::vec3(1, 0, 0));
	return stack.topMatrix() * glm::vec4(0, 0, -1, 0);
}

glm::vec3 CameraObject::getStrafe(glm::vec3 up)
{
	return glm::cross(getFacing(), up);
}

glm::mat4 CameraObject::getProjectionMatrix()
{
	MatrixStack stack;
	stack.pushMatrix();
	stack.loadIdentity();
	stack.perspective(fovy, aspect, zNear, zFar);
	return glm::mat4(stack.topMatrix());
}
