#include <glad/glad.h>

#include "Material.h"

Material::Material(int programIndex) :
	programIndex(programIndex)
{
}

Material::~Material()
{
}

void Material::apply(std::shared_ptr<Scene> scene)
{
	scene->swapToShaderProgram(programIndex);
	scene->blackHole->bind(scene->getCurrentShaderProgram()->getUniform("blackHoleMesh"));
}

SolidColorMaterial::SolidColorMaterial(int programIndex, float r, float g, float b) :
	Material(programIndex),
	r(r),
	g(g),
	b(b)
{
}

void SolidColorMaterial::apply(std::shared_ptr<Scene> scene)
{
	Material::apply(scene);
	auto& currentProgram = scene->getCurrentShaderProgram();
	glUniform3f(currentProgram->getUniform("solidColor"), r, g, b);
}

BlinnPhongMaterial::BlinnPhongMaterial(int programIndex, glm::vec3 matAmb, glm::vec3 matDif, glm::vec3 matSpec, float specIntensity) :
	Material(programIndex),
	matAmb(matAmb),
	matDif(matDif),
	matSpec(matSpec),
	specIntensity(specIntensity)
{
}

void BlinnPhongMaterial::apply(std::shared_ptr<Scene> scene)
{
	Material::apply(scene);
	auto& currentProgram = scene->getCurrentShaderProgram();
	glUniform3f(currentProgram->getUniform("matAmb"), matAmb.r, matAmb.g, matAmb.b);
	glUniform3f(currentProgram->getUniform("matDif"), matDif.r, matDif.g, matDif.b);
	glUniform3f(currentProgram->getUniform("matSpec"), matSpec.r, matSpec.g, matSpec.b);
	glUniform1f(currentProgram->getUniform("specIntensity"), specIntensity);
}

TexBlinnPhongMaterial::TexBlinnPhongMaterial(int programIndex, std::shared_ptr<Texture> texture, float amb, float dif, float spec, float specIntensity) :
	Material(programIndex),
	texture(texture),
	amb(amb),
	dif(dif),
	spec(spec),
	specIntensity(specIntensity)
{
}

void TexBlinnPhongMaterial::apply(std::shared_ptr<Scene> scene)
{
	Material::apply(scene);
	auto& currentProgram = scene->getCurrentShaderProgram();
	texture->bind(currentProgram->getUniform("Texture0"));
	glUniform1f(currentProgram->getUniform("amb"), amb);
	glUniform1f(currentProgram->getUniform("dif"), dif);
	glUniform1f(currentProgram->getUniform("spec"), spec);
	glUniform1f(currentProgram->getUniform("specIntensity"), specIntensity);
}
