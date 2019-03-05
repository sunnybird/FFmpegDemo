package io.bird.sunny.ffmpegapp;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import io.bird.sunny.ffmpegdemo.FFmpengNative;

public class MainActivity extends AppCompatActivity {

    private TextView mTextView;

    private FFmpengNative mFFmpengNative;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mTextView = (TextView) findViewById(R.id.tv1);
        mFFmpengNative = new FFmpengNative();
        mTextView.setText(mFFmpengNative.getVersion());

        mFFmpengNative.help();
    }

    public void onDecodeVideo(View view) {
        mFFmpengNative.decodeVideo("/Share/public/pp.mp4","/Share/public/pp.avi");
    }
}
