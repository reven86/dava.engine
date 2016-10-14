package com.dava.engine;

import android.content.Context;
import android.content.pm.ActivityInfo;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.InputDevice;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.SurfaceHolder;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.util.Log;
import java.lang.reflect.Constructor;

final class DavaSurfaceView extends SurfaceView
                            implements SurfaceHolder.Callback,
                                       View.OnTouchListener,
                                       View.OnKeyListener
{
    protected long windowBackendPointer = 0;
    
    public static native void nativeSurfaceViewOnResume(long windowBackendPointer);
    public static native void nativeSurfaceViewOnPause(long windowBackendPointer);
    public static native void nativeSurfaceViewOnSurfaceCreated(long windowBackendPointer, DavaSurfaceView surfaceView);
    public static native void nativeSurfaceViewOnSurfaceChanged(long windowBackendPointer, Surface surface, int width, int height);
    public static native void nativeSurfaceViewOnSurfaceDestroyed(long windowBackendPointer);
    public static native void nativeSurfaceViewProcessEvents(long windowBackendPointer);
    public static native void nativeSurfaceViewOnTouch(long windowBackendPointer, int action, int touchId, float x, float y);
    public static native void nativeSurfaceViewOnKeyPress(long windowBackendPointer, int action, int keyCode, boolean isRepeated);
    
    public DavaSurfaceView(Context context, long windowBackendPtr)
    {
        super(context);
        getHolder().addCallback(this);
        
        windowBackendPointer = windowBackendPtr;

        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
        setOnTouchListener(this);
        setOnKeyListener(this);
    }

    public Object createNativeControl(String className, long backendPointer)
    {
        try {
            Class<?> clazz = Class.forName(className);
            Constructor<?> ctor = clazz.getConstructor(DavaSurfaceView.class, Long.TYPE);
            return ctor.newInstance(this, backendPointer);
        } catch (Throwable e) {
            Log.e(DavaActivity.LOG_TAG, String.format("DavaSurfaceView.createNativeControl '%s' failed: %s", className, e.toString()));
            return null;
        }
    }

    public void addControl(View control)
    {
        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
                                            FrameLayout.LayoutParams.MATCH_PARENT,
                                            FrameLayout.LayoutParams.MATCH_PARENT);
        ((ViewGroup)getParent()).addView(control, params);
    }

    public void positionControl(View control, float x, float y, float w, float h)
    {
        FrameLayout.LayoutParams params = (FrameLayout.LayoutParams)control.getLayoutParams();
        params.leftMargin = (int)x;
        params.topMargin = (int)y;
        params.width = (int)w;
        params.height = (int)h;
        control.setLayoutParams(params);
    }

    public void removeControl(View control)
    {
        ((ViewGroup)getParent()).removeView(control);
    }

    public void triggerPlatformEvents()
    {
        DavaActivity.commandHandler().sendTriggerProcessEvents(this);
    }

    public void processEvents()
    {
        nativeSurfaceViewProcessEvents(windowBackendPointer);
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
        nativeSurfaceViewOnSurfaceCreated(windowBackendPointer, this);
    }
    
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
    {
        boolean skip = false;
        int requestedOrientation = DavaActivity.instance().getRequestedOrientation();
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
        
        if (DavaActivity.davaMainThread == null)
        {
            // continue initialization of game after creating main window
            DavaActivity.instance().onFinishCreatingMainWindowSurface();
        }
    }
    
    @Override
    public void surfaceDestroyed(SurfaceHolder holder)
    {
        Log.d(DavaActivity.LOG_TAG, "DavaSurface.surfaceDestroyed");
        nativeSurfaceViewOnSurfaceDestroyed(windowBackendPointer);
    }
    
    // View.OnTouchListener interface
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

    // View.OnKeyListener interface
    @Override
    public boolean onKey(View v, int keyCode, KeyEvent event)
    {
        int source = event.getSource();
        int action = event.getAction();

        boolean isRepeated = event.getRepeatCount() > 0;

        if ((source & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD ||
            (source & InputDevice.SOURCE_DPAD) == InputDevice.SOURCE_DPAD)
        {
            // TODO: implement key press handling
        }
        if ((source & InputDevice.SOURCE_KEYBOARD) == InputDevice.SOURCE_KEYBOARD)
        {
            nativeSurfaceViewOnKeyPress(windowBackendPointer, action, keyCode, isRepeated);
        }
        return false;
    }
}
