/*
 * Light.h
 *
 *  Created on: 03.02.2013
 *      Author: ilyakrukov
 */

#ifndef LIGHT_H_
#define LIGHT_H_

#include "Vector.hpp"

class Light {
public:
	vec3 Position;
	vec4 Color;
	Light() ;
	virtual ~Light();
};

#endif /* LIGHT_H_ */
