package com.dava.engine;

import android.os.Bundle;
import android.util.Log;
import android.content.Intent;

import com.dava.engine.DavaActivity;
import com.dava.engine.DavaNotificationProvider;

public class NativeListener extends DavaActivity.ActivityListenerImpl
{
    public static native void nativeNewIntent(String uid);

    void RegisterListener()
    {
        DavaActivity.instance().registerActivityListener(this);
        DavaNotificationProvider.Init(DavaActivity.instance());
    }

    void UnRegisterListener()
    {
        DavaActivity.instance().unregisterActivityListener(this);
    }

    public NativeListener()
    {
        DavaActivity.instance().runOnUiThread(new Runnable() {
            @Override public void run()
            {
                RegisterListener();
            }
        });
        Log.d(DavaActivity.LOG_TAG, "NativeListener.<init> Create class instance.");
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
                DavaNotificationProvider.HideNotification(uid);
                nativeNewIntent(uid);
            }
        }
    }
}
