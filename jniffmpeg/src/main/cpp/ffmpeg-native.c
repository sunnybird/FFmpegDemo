#include <jni.h>
#include <android/log.h>
#include <libavcodec/avcodec.h>

static const char* kTAG = "ffmpeg-native";

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))
#define LOGD(...) \
  ((void)__android_log_print(ANDROID_LOG_DEBUG, kTAG, __VA_ARGS__))


JNIEXPORT jstring JNICALL
Java_io_bird_sunny_ffmpegdemo_FFmpengNative_stringFromJNI(
		JNIEnv *env,
		jobject  obj) {
	LOGD(" the jni called.....");
	const char * config = avcodec_configuration();
	LOGD(" call ffmpeg avcodec_configuration : %s", config);
	return (*env)->NewStringUTF(env,config);
}
