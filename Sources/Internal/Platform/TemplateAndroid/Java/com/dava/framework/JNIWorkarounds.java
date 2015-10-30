package com.dava.framework;

import android.os.Build;
import android.util.Log;
import android.view.View;

public class JNIWorkarounds {

    /**
     * DAVA_WORKAROUND_NEXUS7_ANDROID21
     * Hides and immediately shows GLView.
     * Workaround for Nexus 4 and Nexus 7 (2013) with Android 5+ graphics freezes after loading resources in game.
     * This function should be called from client C++ code after loading many and heavy resources.  
     */
    static public void hideAndShowGLViewOnNexus7()
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP 
                && Build.MODEL != null 
                && (Build.MODEL.contains("Nexus 7") || Build.MODEL.contains("Nexus 4"))) {
            JNIActivity.GetActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    final JNIActivity activity = JNIActivity.GetActivity();
                    if(activity == null)
                        return;
                    final JNISurfaceView surfaceView = activity.GetSurfaceView();
                    if(surfaceView == null)
                        return;
                    
                    Log.d(JNIConst.LOG_TAG, "Reshow GLView");
                    surfaceView.setVisibility(View.GONE);
                    surfaceView.setVisibility(View.VISIBLE);
                }
            });
        }
    }
    
}
