/*
 * SceneLoader.h
 *
 *  Created on: 03.02.2013
 *      Author: ilyakrukov
 */

#ifndef SCENELOADER_H_
#define SCENELOADER_H_

#include <string>
#include <sstream>
#include <fstream>
#include "Scene.h"
#include "ObjLoader.h"
#include "Logger.h"

class SceneLoader {
private:
	ObjLoader m_objLoader;
	std::string m_resourcePath;
public:
	SceneLoader();

	void Load(std::string& fileName, Scene& scene);
	virtual ~SceneLoader();

	void setResourcePath(const std::string& resourcePath) {
		m_resourcePath = resourcePath;
	}
};

#endif /* SCENELOADER_H_ */
