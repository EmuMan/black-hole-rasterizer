#pragma once
#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include <vector>
#include <memory>
#include <glm/gtc/matrix_transform.hpp>

class Scene;
#include "Scene.h"
#include "Texture.h"

class Material
{
public:
	Material(int programIndex);
	virtual ~Material();
	int programIndex;
	virtual void apply(std::shared_ptr<Scene> scene);
};

class SolidColorMaterial : public Material
{
public:
	SolidColorMaterial(int programIndex, float r, float g, float b);
	float r, g, b;
	void apply(std::shared_ptr<Scene> scene) override;
};

class BlinnPhongMaterial : public Material
{
public:
	BlinnPhongMaterial(int programIndex, glm::vec3 matAmb, glm::vec3 matDif, glm::vec3 matSpec, float specIntensity);
	glm::vec3 matAmb, matDif, matSpec;
	float specIntensity;
	void apply(std::shared_ptr<Scene> scene) override;
};

class TexBlinnPhongMaterial : public Material
{
public:
	TexBlinnPhongMaterial(int programIndex, std::shared_ptr<Texture> texture, float amb, float dif, float spec, float specIntensity);
	std::shared_ptr<Texture> texture;
	float amb, dif, spec, specIntensity;
	void apply(std::shared_ptr<Scene> scene) override;
};

#endif
