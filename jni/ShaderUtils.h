//
//  ShaderUtils.h
//  Shadows
//
//  Created by Ilya Kryukov on 15.10.12.
//  Copyright (c) 2012 Ilya Kryukov. All rights reserved.
//

#ifndef Shadows_ShaderUtils_h
#define Shadows_ShaderUtils_h

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <string>
#include <fstream>

#include "Logger.h"

std::string loadShaderFromFile(std::string &filename)
{
	std::ifstream m_stream(filename.c_str());
	std::string line;
	std::string out = "";
	if (m_stream.fail())
	{
		printf("unable to open file\n");
		return out;
	}
	while(std::getline(m_stream, line, '\n'))
	{
		out += line + '\n';
	}
	m_stream.close();
	return out;
}


GLuint BuildShader(const char* source, GLenum shaderType)
{
    GLuint shaderHandle = glCreateShader(shaderType);
    glShaderSource(shaderHandle, 1, &source, 0);
    glCompileShader(shaderHandle);
    
    GLint compileSuccess;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);
    
    if (compileSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
        std::cout << messages;
        LOGI(messages);
        // TODO
        //exit(1);
    }
    
    return shaderHandle;
}

GLuint BuildProgram(const char* vertexShaderSource, const char* fragmentShaderSource)
{
    GLuint vertexShader = BuildShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = BuildShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    
    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vertexShader);
    glAttachShader(programHandle, fragmentShader);
    glLinkProgram(programHandle);
    checkGlError("link");
    GLint linkSuccess;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    checkGlError("get program");
    if (linkSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
        std::cout << messages;
        LOGI(messages);
        //while(1){}
        // TODO
        //exit(1);
    }
    return programHandle;
}

#endif
