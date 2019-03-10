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
     *
     * 转换 h264 裸数据文件为 YUV 文件
     * YUV 文件查看工具 ： XnView
     */
    public native void  decodeVideo(String inFilePath,String outFilePath);

    /**
     * https://www.ffmpeg.org/doxygen/4.0/decode_audio_8c-example.html
     */
    public native void decodeAudio(String inFilePath,String outFilePath);


    public native void help();


    /**
     * 分类 MP4 文件为 h264 视频和 mp3 音频文件
     * 参考 https://blog.csdn.net/leixiaohua1020/article/details/39767055
     * @param inFilePath
     */
    public native void convertMp4toAV(String inFilePath,String vfilepath,String afilepath);


    /**
     * 读取视频文件信息
     * 参考 https://www.ffmpeg.org/doxygen/4.0/avio_reading_8c-example.html
     * @param inFilePath
     */
    public native void avioReading(String inFilePath);

   public native void parseH264(String h264filepath);


}
