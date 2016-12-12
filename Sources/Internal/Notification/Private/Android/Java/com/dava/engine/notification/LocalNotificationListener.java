package com.dava.engine.notification;
import com.dava.engine.DavaActivity;

import android.os.Bundle;
import android.util.Log;
import android.content.Intent;
import com.dava.engine.notification.DavaNotificationProvider;

public class LocalNotificationListener extends DavaActivity.ActivityListenerImpl
{
    protected long localNotificationController = 0;

    public static native void nativeNewIntent(String uid, long controller);

    public LocalNotificationListener(long controller)
    {
        localNotificationController = controller;
        DavaActivity.instance().registerActivityListener(this);
        DavaNotificationProvider.Init(DavaActivity.instance());
        Log.d(DavaActivity.LOG_TAG, "LocalNotificationListener.<init> Create class instance.");
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
                nativeNewIntent(uid, localNotificationController);
            }
        }
    }
}
