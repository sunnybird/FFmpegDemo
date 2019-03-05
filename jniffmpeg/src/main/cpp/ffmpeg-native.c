#include <jni.h>
#include <android/log.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include "stdio.h"
#include <string.h>
#include "decode_video.h"
#include "nativelog.h"




JNIEXPORT jstring JNICALL
Java_io_bird_sunny_ffmpegdemo_FFmpengNative_getVersion(JNIEnv *env, jobject instance) {
	// TODO
    const char * verison = av_version_info();
    const char * config =  avutil_configuration();
    const char *out = (char *) malloc(strlen(verison) + strlen(config) + 1);
    sprintf(out, "%s%s%s", verison,"\n", config);
	return (*env)->NewStringUTF(env,out);

}

JNIEXPORT void JNICALL
Java_io_bird_sunny_ffmpegdemo_FFmpengNative_decodeVideo(JNIEnv *env, jobject instance,
														jstring inFilePath_, jstring outFilePath_) {

	LOGD("decodeVideo");
	const char *inFilePath = (*env)->GetStringUTFChars(env, inFilePath_, 0);
	const char *outFilePath = (*env)->GetStringUTFChars(env, outFilePath_, 0);

	// TODO
	int ret = decode_video(inFilePath,outFilePath);

	(*env)->ReleaseStringUTFChars(env, inFilePath_, inFilePath);
	(*env)->ReleaseStringUTFChars(env, outFilePath_, outFilePath);
}

JNIEXPORT void JNICALL
Java_io_bird_sunny_ffmpegdemo_FFmpengNative_decodeAudio(JNIEnv *env, jobject instance,
														jstring inFilePath_, jstring outFilePath_) {


	const char *inFilePath = (*env)->GetStringUTFChars(env, inFilePath_, 0);
	const char *outFilePath = (*env)->GetStringUTFChars(env, outFilePath_, 0);

	// TODO

	(*env)->ReleaseStringUTFChars(env, inFilePath_, inFilePath);
	(*env)->ReleaseStringUTFChars(env, outFilePath_, outFilePath);
}

JNIEXPORT void JNICALL
Java_io_bird_sunny_ffmpegdemo_FFmpengNative_help(JNIEnv *env, jobject instance) {

	// TODO
	fprintf(stderr, "Error during decoding\n");
}