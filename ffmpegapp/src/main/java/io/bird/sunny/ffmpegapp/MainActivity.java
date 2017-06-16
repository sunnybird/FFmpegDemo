package io.bird.sunny.ffmpegapp;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import io.bird.sunny.ffmpegdemo.FFmpengNative;

public class MainActivity extends AppCompatActivity {

    private TextView mTextView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mTextView = (TextView) findViewById(R.id.tv1);
        FFmpengNative fFmpengNative = new FFmpengNative();
        mTextView.setText(fFmpengNative.stringFromJNI());
    }
}
