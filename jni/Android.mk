LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := Shadows
LOCAL_SRC_FILES := Shadows.cpp IRenderingEngine.hpp MathUtils.h Matrix.hpp ObjLoader.cpp Quaternion.hpp RenderingEngine2.cpp ShaderUtils.h Vector.hpp Light.cpp Scene.cpp SceneLoader.cpp
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2
LOCAL_STATIC_LIBRARIES := android_native_app_glue
include $(BUILD_SHARED_LIBRARY)
$(call import-module,android/native_app_glue)