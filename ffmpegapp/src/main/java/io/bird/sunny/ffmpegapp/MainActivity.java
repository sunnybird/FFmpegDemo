package io.bird.sunny.ffmpegapp;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import java.io.File;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import io.bird.sunny.ffmpegdemo.FFmpengNative;

public class MainActivity extends AppCompatActivity {

    private TextView mTextView;

    private FFmpengNative mFFmpengNative;

    private Executor mExecutor;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mTextView = (TextView) findViewById(R.id.tv1);
        mFFmpengNative = new FFmpengNative();
        mTextView.setText(mFFmpengNative.getVersion());

        mFFmpengNative.help();

        mExecutor = Executors.newCachedThreadPool();

    }

    public void onDecodeVideo(View view) {
        mFFmpengNative.decodeVideo("/data/share/pp.h264","/data/share/pp.yuv");
    }

    public void onGetVideoInfo(View view) {
        File file = new File("/data/share/pp.mp4");
        mFFmpengNative.avioReading(file.getAbsolutePath());
    }

    public void onConvert(View view) {

      new Thread(new Runnable() {
          @Override
          public void run() {
              mFFmpengNative.convertMp4toAV("/data/share/pp.mp4",
                      "/data/share/pp.h264",
                      "/data/share/pp.acc");
          }
      }).start();
    }

    public void onParseH264(View view) {


        mExecutor.execute(new Runnable() {
            @Override
            public void run() {
                mFFmpengNative.parseH264("/data/share/pp.h264");
            }
        });

    }
}
