# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(ffmpeg_DIR ${CMAKE_SOURCE_DIR}/../libffmpeg)

## avcodec
add_library(libavcodec SHARED IMPORTED)
set_target_properties(libavcodec PROPERTIES IMPORTED_LOCATION
    ${ffmpeg_DIR}/lib/${ANDROID_ABI}/libavcodec-57.so)

message(STATUS "CMAKE_SOURCE_DIR ="${CMAKE_SOURCE_DIR})
message(STATUS "ffmpeg_DIR ="${ffmpeg_DIR})

## avdevice
add_library(libavdevice SHARED IMPORTED)
set_target_properties(libavdevice PROPERTIES IMPORTED_LOCATION
    ${ffmpeg_DIR}/lib/${ANDROID_ABI}/libavcodec-57.so)

## avutils
add_library(libavutil SHARED IMPORTED)
set_target_properties(libavutil PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_DIR}/lib/${ANDROID_ABI}/libavutil-55.so)


## avformat
add_library(libavformat SHARED IMPORTED)
set_target_properties(libavformat PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_DIR}/lib/${ANDROID_ABI}/libavformat-57.so)


## swcale
add_library(libswscale SHARED IMPORTED)
set_target_properties(libswscale PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_DIR}/lib/${ANDROID_ABI}/libswscale-4.so)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             ffmpeg-jni-lib
             # Sets the library as a shared library.
             SHARED
             # Provides a relative path to your source file(s).
             src/main/cpp/ffmpeg-native.c
             src/main/cpp/decode_video.c
             src/main/cpp/avio_reading.c
             src/main/cpp/demuxer.c
             src/main/cpp/parseh264.c
        )

## include
target_include_directories(ffmpeg-jni-lib PUBLIC
                           ${ffmpeg_DIR}/include
                           )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       ffmpeg-jni-lib
                       # Links the target library to the log library
                       # included in the NDK.
                       log
                       android
                       libavcodec
                       libavdevice
                       libavutil
                       libavformat
                        libswscale
                        )