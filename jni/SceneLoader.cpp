/*
 * SceneLoader.cpp
 *
 *  Created on: 03.02.2013
 *      Author: ilyakrukov
 */

#include "SceneLoader.h"

SceneLoader::SceneLoader() {
	// TODO Auto-generated constructor stub

}

void SceneLoader::Load(std::string& fileName, Scene& scene) {
	LOGI("Loading scene");
	LOGI(fileName.c_str());
	std::string file = m_resourcePath + std::string("/") + fileName.c_str();
	std::ifstream m_stream(file.c_str());
	std::string line;
	while (std::getline(m_stream, line, '\n')) {
		std::istringstream iss(line);
		std::string token;
		iss >> token;
		if (token == "m") // model
		{
			LOGI("Loading model");
			ObjModel currentModel;
			std::string fileNameModel;
			iss >> fileNameModel;
			file = m_resourcePath + std::string("/") + fileNameModel;
			m_objLoader.Load(file, currentModel);
			currentModel.createVBO();
			vec3 pos;
			iss >> pos.x >> pos.y >> pos.z;
			currentModel.m_Transform = Matrix4<float>::Translate(pos.x, pos.y, pos.z);
			scene.addModel(currentModel);
		}
		else if (token == "l") // light
		{

		}

	}
	m_stream.close();
}

SceneLoader::~SceneLoader() {
	// TODO Auto-generated destructor stub
}

