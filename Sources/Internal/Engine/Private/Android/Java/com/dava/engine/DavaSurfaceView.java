package com.dava.engine;

import android.content.Context;
import android.content.pm.ActivityInfo;
import android.view.MotionEvent;
import android.view.View;
import android.view.InputDevice;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.SurfaceHolder;
import android.util.Log;

public final class DavaSurfaceView extends SurfaceView
                                   implements SurfaceHolder.Callback,
                                              View.OnTouchListener
{
    protected long windowBackendPointer = 0;
    
    public static native void nativeSurfaceViewOnResume(long windowBackendPointer);
    public static native void nativeSurfaceViewOnPause(long windowBackendPointer);
    public static native void nativeSurfaceViewOnSurfaceChanged(long windowBackendPointer, Surface surface, int width, int height);
    public static native void nativeSurfaceViewOnSurfaceDestroyed(long windowBackendPointer);
    public static native void nativeSurfaceViewOnTouch(long windowBackendPointer, int action, int touchId, float x, float y);
    
    public DavaSurfaceView(Context context, long windowBackendPtr)
    {
        super(context);
        getHolder().addCallback(this);
        
        windowBackendPointer = windowBackendPtr;

        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
        setOnTouchListener(this);
    }
    
    public void handleResume()
    {
        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
        setOnTouchListener(this);

        nativeSurfaceViewOnResume(windowBackendPointer);
    }

    public void handlePause()
    {
        nativeSurfaceViewOnPause(windowBackendPointer);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder)
    {
        Log.d(DavaActivity.LOG_TAG, "DavaSurface.surfaceCreated");
    }
    
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
    {
        boolean skip = false;
        int requestedOrientation = DavaActivity.activitySingleton.getRequestedOrientation();
        if (requestedOrientation == ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED)
        {
            // accept any
        }
        else if (requestedOrientation == ActivityInfo.SCREEN_ORIENTATION_PORTRAIT ||
                requestedOrientation == ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT ||
                requestedOrientation == ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT)
        {
            if (w > h)
               skip = true;
        } else if (requestedOrientation == ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE ||
                requestedOrientation == ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE ||
                requestedOrientation == ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE)
        {
            if (w < h)
                skip = true;
        }
        
        if (skip)
        {
            Log.d(DavaActivity.LOG_TAG, String.format("DavaSurface.surfaceChanged: skip w=%d, h=%d", w, h));
            return;
        }

        Log.d(DavaActivity.LOG_TAG, String.format("DavaSurface.surfaceChanged: w=%d, h=%d", w, h));
        nativeSurfaceViewOnSurfaceChanged(windowBackendPointer, holder.getSurface(), w, h);
    }
    
    @Override
    public void surfaceDestroyed(SurfaceHolder holder)
    {
        Log.d(DavaActivity.LOG_TAG, "DavaSurface.surfaceDestroyed");
        nativeSurfaceViewOnSurfaceDestroyed(windowBackendPointer);
    }
    
    @Override
    public boolean onTouch(View v, MotionEvent event) 
    {
        int source = event.getSource();
        int action = event.getActionMasked();
        int pointerCount = event.getPointerCount();

        if (source == InputDevice.SOURCE_TOUCHSCREEN)
        {
            switch (action)
            {
            case MotionEvent.ACTION_MOVE:
                for (int i = 0;i < pointerCount;++i)
                {
                    int pointerId = event.getPointerId(i);
                    float x = event.getX(i);
                    float y = event.getY(i);
                    nativeSurfaceViewOnTouch(windowBackendPointer, MotionEvent.ACTION_MOVE, pointerId, x, y);
                }
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_POINTER_UP:
            case MotionEvent.ACTION_POINTER_DOWN:
            {
                int i = 0;
                if (action == MotionEvent.ACTION_POINTER_UP || action == MotionEvent.ACTION_POINTER_DOWN)
                {
                    i = event.getActionIndex();
                }
                int pointerId = event.getPointerId(i);
                float x = event.getX(i);
                float y = event.getY(i);
                nativeSurfaceViewOnTouch(windowBackendPointer, action, pointerId, x, y);
                break;
            }
            case MotionEvent.ACTION_CANCEL:
                for (int i = 0;i < pointerCount;++i)
                {
                    int pointerId = event.getPointerId(i);
                    float x = event.getX(i);
                    float y = event.getY(i);
                    nativeSurfaceViewOnTouch(windowBackendPointer, MotionEvent.ACTION_UP, pointerId, x, y);
                }
                break;
            }
        }
        return true;
    }
}
