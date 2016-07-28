package com.dava.engine;

import android.content.Context;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.SurfaceHolder;
import android.util.Log;

public final class DavaSurface extends SurfaceView
                               implements SurfaceHolder.Callback
{
    protected boolean isPrimary;
    protected boolean isReady;
    protected long windowBackendPointer;
    
    public static native void nativeSurfaceOnResume(long windowBackendPointer);
    public static native void nativeSurfaceOnPause(long windowBackendPointer);
    public static native void nativeSurfaceChanged(long windowBackendPointer, Surface surface, int width, int height);
    public static native void nativeSurfaceDestroyed(long windowBackendPointer);
    
    public DavaSurface(Context context, long windowBackendPtr, boolean primary)
    {
        super(context);
        getHolder().addCallback(this);
        
        windowBackendPointer = windowBackendPtr;
        isPrimary = primary;

        if (isPrimary)
        {
            setFocusable(true);
            setFocusableInTouchMode(true);
            requestFocus();
        }
    }
    
    public void handleResume()
    {
        if (isPrimary)
        {
            setFocusable(true);
            setFocusableInTouchMode(true);
            requestFocus();
        }
        if (DavaActivity.CALL_NATIVE)
        {
            nativeSurfaceOnResume(windowBackendPointer);
        }
    }

    public void handlePause()
    {
        if (DavaActivity.CALL_NATIVE)
        {
            nativeSurfaceOnPause(windowBackendPointer);
        }
    }

    public void surfaceCreated(SurfaceHolder holder)
    {
        Log.d(DavaActivity.LOG_TAG, "DavaSurface.surfaceCreated");
    }
    
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
    {
        Log.d(DavaActivity.LOG_TAG, String.format("DavaSurface.surfaceChanged: w=%d, h=%d", w, h));

        isReady = true;
        if (DavaActivity.CALL_NATIVE)
        {
            nativeSurfaceChanged(windowBackendPointer, holder.getSurface(), w, h);
        }
    }
    
    public void surfaceDestroyed(SurfaceHolder holder)
    {
        Log.d(DavaActivity.LOG_TAG, "DavaSurface.surfaceDestroyed");

        isReady = false;
        if (DavaActivity.CALL_NATIVE)
        {
            nativeSurfaceDestroyed(windowBackendPointer);
        }
    }
}
