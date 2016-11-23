package com.dava.engine;

import android.os.Bundle;
import android.util.Log;
import android.content.Intent;

import com.dava.engine.DavaActivity;
import com.dava.engine.DavaNotificationProvider;

public class NativeDelegate implements DavaActivity.ActivityListener
{
    public static native void nativeNewIntent(String uid);

    public void RegisterListener()
    {
        DavaActivity.instance().registerActivityListener(this);
        DavaNotificationProvider.Init(DavaActivity.instance());
    }

    public void UnRegisterListener()
    {
        DavaActivity.instance().unregisterActivityListener(this);
    }

    public NativeDelegate()
    {
        Log.d(DavaActivity.LOG_TAG, "NativeDelegate.begin");
        DavaActivity.instance().runOnUiThread(new Runnable() {
            @Override public void run()
            {
                RegisterListener();
            }
        });
        Log.d(DavaActivity.LOG_TAG, "NativeDelegate.<init> Create class instance.");
    }

    void release()
    {
        DavaActivity.instance().runOnUiThread(new Runnable() {
            @Override public void run()
            {
                UnRegisterListener();
            }
        });
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
    }

    @Override
    public void onStart()
    {
    }

    @Override
    public void onResume()
    {
    }

    @Override
    public void onPause()
    {
    }

    @Override
    public void onRestart()
    {
    }

    @Override
    public void onStop()
    {
    }

    @Override
    public void onDestroy()
    {
        DavaActivity.instance().unregisterActivityListener(this);
    }

    @Override
    public void onNewIntent(Intent intent)
    {
        if (null != intent)
        {
            String uid = intent.getStringExtra("uid");
            if (uid != null)
            {
                DavaNotificationProvider.NotificationPressed(uid);
                nativeNewIntent(uid);
            }
        }
    }

}
