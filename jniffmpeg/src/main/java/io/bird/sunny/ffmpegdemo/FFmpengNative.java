package io.bird.sunny.ffmpegdemo;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class FFmpengNative {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("ffmpeg-jni-lib");
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String getVersion();

    /**
     * https://www.ffmpeg.org/doxygen/4.0/decode_video_8c-example.html
     *
     * 分离视频裸数据 格式为 264
     * ffmpeg -i pp.mp4 -codec copy -bsf: h264_mp4toannexb -f h264 pp.264
     */
    public native void  decodeVideo(String inFilePath,String outFilePath);

    /**
     * https://www.ffmpeg.org/doxygen/4.0/decode_audio_8c-example.html
     */
    public native void decodeAudio(String inFilePath,String outFilePath);


    public native void help();


}
