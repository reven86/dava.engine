package com.dava.testbed;

import android.os.Bundle;
import android.util.Log;

import com.dava.engine.DavaActivity;

// Class for testing boot_classes specified in AndroidManifest.xml
public class TestBed implements DavaActivity.ActivityListener
{
    public TestBed()
    {
        Log.d(DavaActivity.LOG_TAG, "TestBed.TestBed");
        DavaActivity.instance().registerActivityListener(this);
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        Log.d(DavaActivity.LOG_TAG, "TestBed.onCreate");
    }

    @Override
    public void onStart()
    {
        Log.d(DavaActivity.LOG_TAG, "TestBed.onStart");
    }

    @Override
    public void onResume()
    {
        Log.d(DavaActivity.LOG_TAG, "TestBed.onResume");
    }

    @Override
    public void onPause()
    {
        Log.d(DavaActivity.LOG_TAG, "TestBed.onPause");
    }

    @Override
    public void onRestart()
    {
        Log.d(DavaActivity.LOG_TAG, "TestBed.onRestart");
    }

    @Override
    public void onStop()
    {
        Log.d(DavaActivity.LOG_TAG, "TestBed.onStop");
    }

    @Override
    public void onDestroy()
    {
        Log.d(DavaActivity.LOG_TAG, "TestBed.onDestroy");
        DavaActivity.instance().unregisterActivityListener(this);
    }
}
