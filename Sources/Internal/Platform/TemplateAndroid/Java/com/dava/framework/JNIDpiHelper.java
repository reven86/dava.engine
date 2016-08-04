package com.dava.framework;

import com.dava.engine.DavaActivity;

import android.util.DisplayMetrics;

public class JNIDpiHelper {
    public static int GetScreenDPI()
    {
        DisplayMetrics dm = new DisplayMetrics();
        if (DavaActivity.activitySingleton != null)
        {
            DavaActivity.activitySingleton.getWindowManager().getDefaultDisplay().getMetrics(dm);
        }
        else
        {
            JNIActivity.GetActivity().getWindowManager().getDefaultDisplay().getMetrics(dm);
        }
        return (int)dm.densityDpi;
    }
}
