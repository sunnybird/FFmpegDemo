//
// Created by fengjl on 2019/3/5.
//

#ifndef FFMPEGDEMO_NATIVELOG_H
#define FFMPEGDEMO_NATIVELOG_H
static const char* kTAG = "ffmpeg-native";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))
#define LOGD(...) \
  ((void)__android_log_print(ANDROID_LOG_DEBUG, kTAG, __VA_ARGS__))

#endif //FFMPEGDEMO_NATIVELOG_H
