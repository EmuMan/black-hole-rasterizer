#include <iostream>

#include "Model.h"
#include "Program.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>
#include <glad/glad.h>

Model::Model() :
	shapes(std::vector<std::shared_ptr<Shape>>()),
	flipNormals(false),
	useBlackHole(true)
{
}

Model::Model(const std::string& path) :
	Model()
{
	std::vector<tinyobj::shape_t> TOshapes;
	std::vector<tinyobj::material_t> objMaterials;
	std::string errStr;

	//load in the mesh and make the shape(s)
	bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, path.c_str());

	if (!rc) {
		std::cerr << errStr << std::endl;
	}
	else {
		for (auto& toShape : TOshapes)
		{
			auto shape = std::make_shared<Shape>(true);
			shape->createShape(toShape);
			shape->measure();
			shape->init();
			addShape(shape);
		}
	}
}

Model::~Model()
{
}

void Model::draw(const std::shared_ptr<Program> prog) const
{
	glUniform1i(prog->getUniform("flipNormals"), flipNormals);
	glUniform1i(prog->getUniform("useBlackHole"), useBlackHole);
	glUniform1i(prog->getUniform("blackHoleSecondary"), false);
	for (auto& shape : shapes)
	{
		shape->draw(prog);
	}
	glUniform1i(prog->getUniform("blackHoleSecondary"), true);
	for (auto& shape : shapes)
	{
		shape->draw(prog);
	}
}

void Model::addShape(std::shared_ptr<Shape> shape)
{
	shapes.push_back(shape);
}

glm::vec3 Model::getMin()
{
	auto min = glm::vec3(0.0);

	for (auto& shape : shapes)
	{
		if (shape->min.x < min.x)
			min.x = shape->min.x;
		if (shape->min.y < min.y)
			min.y = shape->min.y;
		if (shape->min.z < min.z)
			min.z = shape->min.z;
	}

	return min;
}

glm::vec3 Model::getMax()
{
	auto max = glm::vec3(0.0);

	for (auto& shape : shapes)
	{
		if (shape->max.x < max.x)
			max.x = shape->max.x;
		if (shape->max.y < max.y)
			max.y = shape->max.y;
		if (shape->max.z < max.z)
			max.z = shape->max.z;
	}

	return max;
}
