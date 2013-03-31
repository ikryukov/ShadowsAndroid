/*
 * Scene.h
 *
 *  Created on: 03.02.2013
 *      Author: ilyakrukov
 */

#ifndef SCENE_H_
#define SCENE_H_


#include <vector>

#include "ObjLoader.h"
#include "Light.h"

class Scene {
	std::vector<ObjModel> m_models;
	std::vector<Light> m_lights;
public:

	Scene();
	virtual ~Scene();

	void addModel(const ObjModel& model) {
		m_models.push_back(model);
	}

	const std::vector<Light>& getLights() const {
		return m_lights;
	}

	const std::vector<ObjModel>& getModels() const {
		return m_models;
	}
};

#endif /* SCENE_H_ */
