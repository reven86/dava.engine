package com.dava.engine;

import android.content.Context;
import android.view.SurfaceView;
import android.view.SurfaceHolder;
import android.util.Log;

public final class DavaSurface extends SurfaceView
                               implements SurfaceHolder.Callback
{
    private DavaActivity activity;
    
    public DavaSurface(Context context, DavaActivity activity)
    {
        super(context);
        getHolder().addCallback(this);
        this.activity = activity;
    }
    
    public void surfaceCreated(SurfaceHolder holder)
    {
        Log.d(DavaActivity.LOG_TAG, "DavaSurface.surfaceCreated");
    }
    
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
    {
        Log.d(DavaActivity.LOG_TAG, String.format("DavaSurface.surfaceChanged: w=%d, h=%d", w, h));
    }
    
    public void surfaceDestroyed(SurfaceHolder holder)
    {
        Log.d(DavaActivity.LOG_TAG, "DavaSurface.surfaceDestroyed");
    }
}
