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
    public native String stringFromJNI();
}
