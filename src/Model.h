#pragma once
#ifndef _MODEL_H_
#define _MODEL_H_

#include <vector>
#include <memory>
#include "Shape.h"

class Model
{
public:
	Model();
	Model(const std::string& path);
	virtual ~Model();
	void draw(const std::shared_ptr<Program> prog) const;
	void addShape(std::shared_ptr<Shape> shape);
	glm::vec3 getMin();
	glm::vec3 getMax();
	std::vector<std::shared_ptr<Shape>> shapes;
	bool flipNormals;
	bool useBlackHole;
};

#endif
