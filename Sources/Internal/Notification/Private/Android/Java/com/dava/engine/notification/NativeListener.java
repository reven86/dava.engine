package com.dava.engine.notification;
import com.dava.engine.DavaActivity;

import android.os.Bundle;
import android.util.Log;
import android.content.Intent;

import com.dava.engine.DavaActivity;
import com.dava.engine.notification.DavaNotificationProvider;

public class NativeListener extends DavaActivity.ActivityListenerImpl
{
    public static native void nativeNewIntent(String uid);

    public NativeListener()
    {
        DavaActivity.instance().registerActivityListener(this);
        DavaNotificationProvider.Init(DavaActivity.instance());
        Log.d(DavaActivity.LOG_TAG, "NativeListener.<init> Create class instance.");
    }

    void release()
    {
        DavaActivity.instance().unregisterActivityListener(this);
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
