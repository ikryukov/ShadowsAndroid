#include <jni.h>
#include <errno.h>
#include <EGL/egl.h>
//#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <android/log.h>
#include <android_native_app_glue.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Logger.h"

#include "IRenderingEngine.hpp"

IRenderingEngine* render = CreateRenderer2();
EGLDisplay display;
EGLSurface surface;

bool defaultEGLChooser(EGLDisplay disp, EGLConfig& bestConfig)
{

	EGLint count = 0;
	if (!eglGetConfigs(disp, NULL, 0, &count))
	{
		LOGE("defaultEGLChooser cannot query count of all configs");
		return false;
	}

	LOGD("Config count = %d", count);

	EGLConfig* configs = new EGLConfig[count];
	if (!eglGetConfigs(disp, configs, count, &count))
	{
		LOGE("defaultEGLChooser cannot query all configs");
		return false;
	}

	int bestMatch = 1<<30;
	int bestIndex = -1;

	int i;
	for (i = 0; i < count; i++)
	{
		int match = 0;
		EGLint surfaceType = 0;
		EGLint blueBits = 0;
		EGLint greenBits = 0;
		EGLint redBits = 0;
		EGLint alphaBits = 0;
		EGLint depthBits = 0;
		EGLint stencilBits = 0;
		EGLint renderableFlags = 0;

		eglGetConfigAttrib(disp, configs[i], EGL_SURFACE_TYPE, &surfaceType);
		eglGetConfigAttrib(disp, configs[i], EGL_BLUE_SIZE, &blueBits);
		eglGetConfigAttrib(disp, configs[i], EGL_GREEN_SIZE, &greenBits);
		eglGetConfigAttrib(disp, configs[i], EGL_RED_SIZE, &redBits);
		eglGetConfigAttrib(disp, configs[i], EGL_ALPHA_SIZE, &alphaBits);
		eglGetConfigAttrib(disp, configs[i], EGL_DEPTH_SIZE, &depthBits);
		eglGetConfigAttrib(disp, configs[i], EGL_STENCIL_SIZE, &stencilBits);
		eglGetConfigAttrib(disp, configs[i], EGL_RENDERABLE_TYPE, &renderableFlags);
		LOGD("Config[%d]: R%dG%dB%dA%d D%dS%d Type=%04x Render=%04x",
			i, redBits, greenBits, blueBits, alphaBits, depthBits, stencilBits, surfaceType, renderableFlags);

		if ((surfaceType & EGL_WINDOW_BIT) == 0)
			continue;
		if ((renderableFlags & EGL_OPENGL_ES2_BIT) == 0)
			continue;
		if (depthBits < 16)
			continue;
		if ((redBits < 5) || (greenBits < 6) || (blueBits < 5))
			continue;

		int penalty = depthBits - 16;
		match += penalty * penalty;
		penalty = redBits - 5;
		match += penalty * penalty;
		penalty = greenBits - 6;
		match += penalty * penalty;
		penalty = blueBits - 5;
		match += penalty * penalty;
		penalty = alphaBits;
		match += penalty * penalty;
		penalty = stencilBits;
		match += penalty * penalty;

		if ((match < bestMatch) || (bestIndex == -1))
		{
			bestMatch = match;
			bestIndex = i;
			LOGD("Config[%d] is the new best config", i, configs[i]);
		}
	}

	if (bestIndex < 0)
	{
		delete[] configs;
		return false;
	}

	bestConfig = configs[bestIndex];
	delete[] configs;

	return true;
}

bool setupGraphics(struct android_app* state) {

	const EGLint attribs[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE,
			8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_NONE };
	EGLint contextAttrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

	EGLint w, h, dummy, format;
	EGLint numConfigs;
	EGLConfig config;

	EGLContext context;

	display = eglGetDisplay(EGL_DEFAULT_DISPLAY );

	eglInitialize(display, 0, 0);

	defaultEGLChooser(display, config);

	//eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	context = eglCreateContext(display, config, NULL, contextAttrs);

	ANativeWindow_setBuffersGeometry(state->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, state->window, NULL);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	printGLString("Version", GL_VERSION);
	printGLString("Vendor", GL_VENDOR);
	printGLString("Renderer", GL_RENDERER);
	printGLString("Extensions", GL_EXTENSIONS);
	LOGI("setupGraphics(%d, %d)", w, h);
	render->SetPivotPoint(w * 0.5f, h * 0.5f);
	std::string resourcePath("/sdcard");
	render->SetResourcePath(resourcePath);
	render->Initialize(w, h);
	checkGlError("glViewport");
	return true;
}

void renderFrame() {
	//LOGI("renderFrame");
	render->Render();
	eglSwapBuffers(display, surface);
}

static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
	switch (cmd) {
	case APP_CMD_SAVE_STATE:
		// The system has asked us to save our current state.  Do so.
		break;
	case APP_CMD_INIT_WINDOW:
		// The window is being shown, get it ready.
		if (app->window != NULL) {
			setupGraphics(app);
//			renderFrame();
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// The window is being hidden or closed, clean it up.
		break;
	case APP_CMD_GAINED_FOCUS:
		// When our app gains focus, we start monitoring the accelerometer.
		break;
	case APP_CMD_LOST_FOCUS:
		// When our app loses focus, we stop monitoring the accelerometer.
		// This is to avoid consuming battery while not being used.
		// Also stop animating.
		break;
	}
}
void android_main(struct android_app* state) {
	// Make sure glue isn't stripped.
	app_dummy();

	state->onAppCmd = engine_handle_cmd;

	while (1) {
		int ident;
		int fdesc;
		int events;
		struct android_poll_source* source;

		while ((ident = ALooper_pollAll(0, &fdesc, &events, (void**) &source))
				>= 0) {
			// process this event
			if (source)
				source->process(state, source);

		}
		renderFrame();
	}
}

