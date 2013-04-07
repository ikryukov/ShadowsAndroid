#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "Logger.h"
#include "IRenderingEngine.hpp"
#include "Matrix.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string>
#include "ObjLoader.h"
#include "MathUtils.h"
#include "ShaderUtils.h"
#include "Light.h"
#include "Scene.h"
#include "SceneLoader.h"

#include <assert.h>

#ifndef GL_TEXTURE_COMPARE_MODE_EXT
#   define GL_TEXTURE_COMPARE_MODE_EXT      0x884C
#endif
#ifndef GL_TEXTURE_COMPARE_FUNC_EXT
#   define GL_TEXTURE_COMPARE_FUNC_EXT      0x884D
#endif
#ifndef GL_COMPARE_REF_TO_TEXTURE_EXT
#   define GL_COMPARE_REF_TO_TEXTURE_EXT    0x884E
#endif

#ifndef VSM
#define VSM
#endif

using namespace std;

class RenderingEngine2: public IRenderingEngine {
public:
	RenderingEngine2();
	void Initialize(int width, int height);
	void Render();
	void UpdateAnimation(float timeStep) {
	}
	void OnRotate(DeviceOrientation newOrientation) {
	}
	void OnFingerUp(vec2 location);
	void OnFingerDown(vec2 location);
	void OnFingerMove(vec2 oldLocation, vec2 newLocation);
	void SetResourcePath(std::string& path);
	void SetPivotPoint(float x, float y);
private:

	void shadowPass();
	void mainPass();
	void fullscreenQuad();

	bool useVSM;

	GLfloat m_rotationAngle;
	GLfloat m_scale;
	vec2 m_pivotPoint;

	GLuint m_simpleProgram;
	GLuint m_quadProgram;
	GLuint m_shadowMapProgram;
	GLuint m_vsmProgram;

	GLuint m_framebuffer;
	GLuint m_colorbufferTexture;
	GLuint m_colorRenderbuffer;
	GLuint m_depthRenderbuffer;
	GLuint m_textureShadow;
	GLuint m_textureShadow2;
	GLuint m_fboShadow;
	GLuint m_fboShadow2;
	GLuint m_rboShadow;
	// uniforms for main pass
	GLint uniformModelviewMain;
	GLint uniformProjectionMain;

	// attribs for main pass
	GLint attribPositionMain;

	// uniforms for shadow pass
	GLint uniformModelviewShadow;
	GLint uniformProjectionShadow;
	GLint uniformLightMatrixShadow;
	GLint uniformShadowMapTextureShadow;
	GLint uniformTransformShadow;

	// attribs for main pass
	GLint attribPositionShadow;
	GLint attribNormalShadow;
	GLint attribColorShadow;
	GLint attribTexCoordShadow;

	mat4 rotation;
	mat4 scale;
	mat4 translation;
	mat4 modelviewMatrix;
	mat4 projectionMatrix;

	mat4 lightProjectionMatrix;
	mat4 lightModelviewMatrix;

	vec2 screen;
	ivec2 shadowmapSize;
	ObjModel teapot;
	ObjModel palm;
	ObjModel plane;
	ObjModel quad;
	//vector<ObjModel> objects;

	string resourcePath;
	Light m_light;
	Scene m_Scene;
	SceneLoader m_SceneLoader;
};

void RenderingEngine2::SetResourcePath(std::string& path) {
	resourcePath = path;
}

void RenderingEngine2::SetPivotPoint(float x, float y) {
	m_pivotPoint = vec2(x, y);
}

IRenderingEngine* CreateRenderer2() {
	return new RenderingEngine2();
}

RenderingEngine2::RenderingEngine2() :
		m_rotationAngle(0), m_scale(1), useVSM(true){
}

void RenderingEngine2::Initialize(int width, int height) {
	printf("x = %f, y = %f\n", m_pivotPoint.x, m_pivotPoint.y);
	string filename = string("teapot.obj");
	string file = resourcePath + string("/") + filename;
	ObjLoader loader;
	m_SceneLoader.setResourcePath(resourcePath + std::string("/"));
	file = std::string("scene.txt");
	m_SceneLoader.Load(file, m_Scene);

//	loader.Load(file, teapot);
//	teapot.createVBO();
//	checkGlError("teapot");

//	file = resourcePath + string("/") + string("tree.obj");
//	loader.Load(file, palm);
//	palm.createVBO();

//	file = resourcePath + string("/") + string("plane.obj");
//	loader.Load(file, plane);
//	plane.createVBO();

	file = resourcePath + string("/") + string("quad.obj");
	loader.Load(file, quad);
	quad.createVBO();

//	objects.push_back(plane);
//	objects.push_back(teapot);
	//objects.push_back(palm);

	// Create & bind the color buffer so that the caller can allocate its space.
	glGenRenderbuffers(1, &m_colorRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderbuffer);
	checkGlError("bind color buffer");
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB565, width, height);

	checkGlError("color buffer");
//	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	// Create the depth buffer.
	glGenRenderbuffers(1, &m_depthRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
	checkGlError("depth buffer");
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	// Create the framebuffer object; attach the depth and color buffers.
	glGenFramebuffers(1, &m_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorbufferTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colorRenderbuffer);
	checkGlError("GL_COLOR_ATTACHMENT0");
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);
	checkGlError("GL_DEPTH_ATTACHMENT");
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		LOGI("Framebuffer is not complete");
		switch (status) {
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			LOGI("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
			LOGI("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			LOGI("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			LOGI("GL_FRAMEBUFFER_UNSUPPORTED");
			break;
		default:
			break;
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	checkGlError("frame buffer");

	glViewport(0, 0, width, height);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	screen.x = width;
	screen.y = height;

	shadowmapSize.x = 512 * 2;
	shadowmapSize.y = 512 * 2;

	string shaderPath = resourcePath + string("/Simple.vert");
	string vertexShaderSource = loadShaderFromFile(shaderPath);
	shaderPath = resourcePath + string("/Simple.frag");
	string fragmentShaderSource = loadShaderFromFile(shaderPath);

	m_simpleProgram = BuildProgram(vertexShaderSource.c_str(),
			fragmentShaderSource.c_str());
	checkGlError("simple program");
	shaderPath = resourcePath + string("/Quad.vert");
	vertexShaderSource = loadShaderFromFile(shaderPath);
	shaderPath = resourcePath + string("/Quad.frag");
	fragmentShaderSource = loadShaderFromFile(shaderPath);

	m_quadProgram = BuildProgram(vertexShaderSource.c_str(),
			fragmentShaderSource.c_str());

	shaderPath = resourcePath + string("/Shadow.vert");
	vertexShaderSource = loadShaderFromFile(shaderPath);
	shaderPath = resourcePath + string("/Shadow.frag");
	fragmentShaderSource = loadShaderFromFile(shaderPath);

	m_shadowMapProgram = BuildProgram(vertexShaderSource.c_str(),
			fragmentShaderSource.c_str());
	checkGlError("shadowmap program");

	shaderPath = resourcePath + string("/ShadowVSM.vert");
	vertexShaderSource = loadShaderFromFile(shaderPath);
	shaderPath = resourcePath + string("/ShadowVSM.frag");
	fragmentShaderSource = loadShaderFromFile(shaderPath);

	m_vsmProgram = BuildProgram(vertexShaderSource.c_str(), fragmentShaderSource.c_str());
	checkGlError("VSM program");

	// set uniforms
	uniformModelviewMain = glGetUniformLocation(m_simpleProgram, "Modelview");
	uniformProjectionMain = glGetUniformLocation(m_simpleProgram, "Projection");
	checkGlError("getUniform Simple");

	uniformModelviewShadow = glGetUniformLocation(m_shadowMapProgram, "Modelview");
	checkGlError("getUniform Modelview");

	uniformProjectionShadow = glGetUniformLocation(m_shadowMapProgram, "Projection");
	uniformLightMatrixShadow = glGetUniformLocation(m_shadowMapProgram, "lightMatrix");
	uniformShadowMapTextureShadow = glGetUniformLocation(m_shadowMapProgram, "shadowMapTex");
	uniformTransformShadow = glGetUniformLocation(m_shadowMapProgram, "Transform");
	checkGlError("getUniform");
	// set attribs
	attribPositionMain = glGetAttribLocation(m_simpleProgram, "Position");

	attribPositionShadow = glGetAttribLocation(m_shadowMapProgram, "Position");
	attribColorShadow = glGetAttribLocation(m_shadowMapProgram, "SourceColor");
	attribNormalShadow = glGetAttribLocation(m_shadowMapProgram, "Normal");
	attribTexCoordShadow = glGetAttribLocation(m_shadowMapProgram, "TexCoord");
	checkGlError("getAttrib");
	m_light.Position = vec3(10, 10, 10);

	//Create the shadow map texture
	glGenTextures(1, &m_textureShadow);
	glBindTexture(GL_TEXTURE_2D, m_textureShadow);
	checkGlError("bind texture");
	// Create the depth texture.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowmapSize.x, shadowmapSize.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
	checkGlError("image2d");
	// Set the textures parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// create frame buffer object for shadow pass
	glGenFramebuffers(1, &m_fboShadow);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboShadow);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_textureShadow, 0);
	checkGlError("shadowmaptexture");
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		LOGI("init: ");
		LOGI("failed to make complete framebuffer object %x\n", status);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (useVSM) {
		//Create the shadow map texture
		glGenTextures(1, &m_textureShadow2);
		glBindTexture(GL_TEXTURE_2D, m_textureShadow2);
		checkGlError("bind texture");
		// Create the depth texture.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, shadowmapSize.x,
				shadowmapSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		checkGlError("image2d");
		// Set the textures parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glGenFramebuffers(1, &m_fboShadow2);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fboShadow2);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D, m_textureShadow2, 0);
		checkGlError("shadowmaptexture2");
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			LOGE("init: ");
			LOGE("failed to make complete framebuffer object %d\n", status);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


	checkGlError("end of init");
}

void RenderingEngine2::shadowPass() {
	GLenum status;
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboShadow);
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		LOGE("Shadow pass: ");
		LOGE("failed to make complete framebuffer object %x\n", status);
	}
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	lightProjectionMatrix = VerticalFieldOfView(90.0,
			(shadowmapSize.x + 0.0) / shadowmapSize.y, 0.1, 100.0);
	lightModelviewMatrix = LookAt(vec3(0, 4, 7), vec3(0.0, 0.0, 0.0), vec3(0, -7, 4));
	glCullFace(GL_FRONT);
	glUseProgram(m_simpleProgram);
	glUniformMatrix4fv(uniformProjectionMain, 1, 0,
			lightProjectionMatrix.Pointer());
	glUniformMatrix4fv(uniformModelviewMain, 1, 0,
			lightModelviewMatrix.Pointer());
	glViewport(0, 0, shadowmapSize.x - 2, shadowmapSize.y - 2);

	GLsizei stride = sizeof(Vertex);
	const vector<ObjModel>& objects = m_Scene.getModels();
	const GLvoid* bodyOffset = 0;
	for (int i = 0; i < objects.size(); ++i) {
		lightModelviewMatrix = objects[i].m_Transform * LookAt(vec3(0, 4, 7), vec3(0.0, 0.0, 0.0), vec3(0, -7, 4));
		glUniformMatrix4fv(uniformModelviewMain, 1, 0, lightModelviewMatrix.Pointer());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[i].m_indexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, objects[i].m_vertexBuffer);

		glVertexAttribPointer(attribPositionMain, 3, GL_FLOAT, GL_FALSE, stride,
				(GLvoid*) offsetof(Vertex, Position));

		glEnableVertexAttribArray(attribPositionMain);

		glDrawElements(GL_TRIANGLES, objects[i].m_indexCount, GL_UNSIGNED_SHORT,
				bodyOffset);

		glDisableVertexAttribArray(attribPositionMain);
	}
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCullFace(GL_BACK);
}

void RenderingEngine2::mainPass() {
	glClearColor(0.5f, 0.5f, 0.5f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	modelviewMatrix = scale * rotation * translation
			* LookAt(vec3(0, 8, 7), vec3(0.0, 0.0, 0.0), vec3(0, 7, -8));
	lightModelviewMatrix = LookAt(vec3(0, 4, 7), vec3(0.0, 0.0, 0.0), vec3(0, -7, 4));

	projectionMatrix = VerticalFieldOfView(45.0, (screen.x + 0.0) / screen.y, 0.1, 100.0);
	mat4 offsetLight = mat4::Scale(0.5f) * mat4::Translate(0.5, 0.5, 0.5);
	mat4 lightMatrix = lightModelviewMatrix * lightProjectionMatrix	* offsetLight;
	glUseProgram(m_shadowMapProgram);
	glUniformMatrix4fv(uniformLightMatrixShadow, 1, 0, lightMatrix.Pointer());
	glUniformMatrix4fv(uniformProjectionShadow, 1, 0, projectionMatrix.Pointer());
	glUniformMatrix4fv(uniformModelviewShadow, 1, 0, modelviewMatrix.Pointer());

	glViewport(0, 0, screen.x, screen.y);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureShadow);
	glUniform1i(uniformShadowMapTextureShadow, 0);

	GLsizei stride = sizeof(Vertex);
	const vector<ObjModel>& objects = m_Scene.getModels();
	const GLvoid* bodyOffset = 0;
	for (int i = 0; i < objects.size(); ++i) {
		modelviewMatrix = scale * rotation * translation * LookAt(vec3(0, 8, 7), vec3(0.0, 0.0, 0.0), vec3(0, 7, -8));
		glUniformMatrix4fv(uniformTransformShadow, 1, 0, objects[i].m_Transform.Pointer());
		glUniformMatrix4fv(uniformModelviewShadow, 1, 0, modelviewMatrix.Pointer());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[i].m_indexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, objects[i].m_vertexBuffer);

		glVertexAttribPointer(attribPositionShadow, 3, GL_FLOAT, GL_FALSE,
				stride, (GLvoid*) offsetof(Vertex, Position));
		glVertexAttribPointer(attribColorShadow, 4, GL_FLOAT, GL_FALSE, stride,
				(GLvoid*) offsetof(Vertex, Color));
		glVertexAttribPointer(attribNormalShadow, 3, GL_FLOAT, GL_FALSE, stride,
				(GLvoid*) offsetof(Vertex, Normal));
		glVertexAttribPointer(attribTexCoordShadow, 2, GL_FLOAT, GL_FALSE,
				stride, (GLvoid*) offsetof(Vertex, TexCoord));

		glEnableVertexAttribArray(attribPositionShadow);
		glEnableVertexAttribArray(attribNormalShadow);
		glEnableVertexAttribArray(attribColorShadow);
		glEnableVertexAttribArray(attribTexCoordShadow);

		glDrawElements(GL_TRIANGLES, objects[i].m_indexCount, GL_UNSIGNED_SHORT,
				bodyOffset);

		glDisableVertexAttribArray(attribColorShadow);
		glDisableVertexAttribArray(attribPositionShadow);
		glDisableVertexAttribArray(attribNormalShadow);
		glDisableVertexAttribArray(attribTexCoordShadow);
	}
}

void RenderingEngine2::fullscreenQuad() {
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.9f, 0.5f, 0.5f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, screen.x, screen.y);
	glUseProgram(m_quadProgram);

	GLint uniformShadowMapTextureQuad = glGetUniformLocation(m_quadProgram,	"sShadow");
	GLint attribPositionQuad = glGetAttribLocation(m_quadProgram, "Position");
	GLint attribColorQuad = glGetAttribLocation(m_quadProgram, "SourceColor");
	GLint attribNormalQuad = glGetAttribLocation(m_quadProgram, "Normal");
	GLint attribTexCoordQuad = glGetAttribLocation(m_quadProgram, "TexCoord");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureShadow);
	glUniform1i(uniformShadowMapTextureQuad, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad.m_indexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad.m_vertexBuffer);


	GLsizei stride = sizeof(Vertex);
	const GLvoid* bodyOffset = 0;

	glVertexAttribPointer(attribPositionQuad, 3, GL_FLOAT, GL_FALSE,
			stride, (GLvoid*) offsetof(Vertex, Position));
	glVertexAttribPointer(attribColorQuad, 4, GL_FLOAT, GL_FALSE, stride,
			(GLvoid*) offsetof(Vertex, Color));
	glVertexAttribPointer(attribNormalQuad, 3, GL_FLOAT, GL_FALSE, stride,
			(GLvoid*) offsetof(Vertex, Normal));
	glVertexAttribPointer(attribTexCoordQuad, 2, GL_FLOAT, GL_FALSE,
			stride, (GLvoid*) offsetof(Vertex, TexCoord));
	glEnableVertexAttribArray(attribPositionQuad);
	glEnableVertexAttribArray(attribNormalQuad);
	glEnableVertexAttribArray(attribColorQuad);
	glEnableVertexAttribArray(attribTexCoordQuad);
	glDrawElements(GL_TRIANGLES, quad.m_indexCount, GL_UNSIGNED_SHORT,
			bodyOffset);

	glDisableVertexAttribArray(attribColorQuad);
	glDisableVertexAttribArray(attribPositionQuad);
	glDisableVertexAttribArray(attribNormalQuad);
	glDisableVertexAttribArray(attribTexCoordQuad);
}

void RenderingEngine2::Render() {
	m_rotationAngle += 1.0f;
	if (m_rotationAngle > 360.0f)
		m_rotationAngle = 0.0f;
	rotation = mat4::RotateY(m_rotationAngle);
	scale = mat4::Scale(m_scale);
	translation = mat4::Translate(0, 0, 0);

	// Shadow pass
	shadowPass();
//	fullscreenQuad();
	// Main pass
	mainPass();
}

void RenderingEngine2::OnFingerUp(vec2 location) {
	m_scale = 1.0f;
}

void RenderingEngine2::OnFingerDown(vec2 location) {
	m_scale = 1.5f;
	printf("TOUCH x = %f, y = %f\n", location.x, location.y);
	OnFingerMove(location, location);
}

void RenderingEngine2::OnFingerMove(vec2 previous, vec2 location) {
	vec2 direction = vec2(location - m_pivotPoint).Normalized();

	// Flip the Y axis because pixel coords increase towards the bottom.
	direction.y = -direction.y;

	m_rotationAngle = std::acos(direction.y) * 180.0f / 3.14159f;
	if (direction.x > 0)
		m_rotationAngle = -m_rotationAngle;
}
