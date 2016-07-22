package com.dava.engine;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

public class DavaActivity extends Activity
{
    public static String LOG_TAG = "DAVA";

    protected void onCreate(Bundle savedInstanceState)
    {
        Log.d(LOG_TAG, "DavaActivity.onCreate");
    }

    protected void onStart()
    {
        Log.d(LOG_TAG, "DavaActivity.onStart");
    }

    protected void onResume()
    {
        Log.d(LOG_TAG, "DavaActivity.onResume");
    }

    protected void onPause()
    {
        Log.d(LOG_TAG, "DavaActivity.onPause");
    }

    protected void onStop()
    {
        Log.d(LOG_TAG, "DavaActivity.onStop");
    }

    protected void onRestart()
    {
        Log.d(LOG_TAG, "DavaActivity.onRestart");
    }

    protected void onDestroy()
    {
        Log.d(LOG_TAG, "DavaActivity.onDestroy");
    }
}
