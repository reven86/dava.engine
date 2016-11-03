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
import android.util.DisplayMetrics;
import java.lang.reflect.Constructor;

final class DavaSurfaceView extends SurfaceView
                            implements SurfaceHolder.Callback,
                                       View.OnTouchListener,
/* uncomment after multidex enabled
                                       View.OnGenericMotionListener,
*/
                                       View.OnKeyListener
{
    protected long windowBackendPointer = 0;
    
    public static native void nativeSurfaceViewOnResume(long windowBackendPointer);
    public static native void nativeSurfaceViewOnPause(long windowBackendPointer);
    public static native void nativeSurfaceViewOnSurfaceCreated(long windowBackendPointer, DavaSurfaceView surfaceView);
    public static native void nativeSurfaceViewOnSurfaceChanged(long windowBackendPointer, Surface surface, int width, int height, int surfaceWidth, int surfaceHeight, int dpi);
    public static native void nativeSurfaceViewOnSurfaceDestroyed(long windowBackendPointer);
    public static native void nativeSurfaceViewProcessEvents(long windowBackendPointer);
/* uncomment after multidex enabled
    public static native void nativeSurfaceViewOnMouseEvent(long windowBackendPointer, int action, int buttonId, float x, float y, float deltaX, float deltaY, int modifierKeys);
    public static native void nativeSurfaceViewOnTouchEvent(long windowBackendPointer, int action, int touchId, float x, float y, int modifierKeys);
    public static native void nativeSurfaceViewOnKeyEvent(long windowBackendPointer, int action, int keyCode, int unicodeChar, int modifierKeys, boolean isRepeated);
    public static native void nativeSurfaceViewOnGamepadButton(long windowBackendPointer, int deviceId, int action, int keyCode);
    public static native void nativeSurfaceViewOnGamepadMotion(long windowBackendPointer, int deviceId, int axis, float value);
*/
    
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
/* uncomment after multidex enabled
        setOnGenericMotionListener(this);
*/
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

    public int getDpi()
    {
        final DisplayMetrics dm = new DisplayMetrics();
        DavaActivity.instance().getWindowManager().getDefaultDisplay().getMetrics(dm);

        // Use dm.densityDpi because it returns DPI that used by system for UI scaling.
        // Values of dm.(x|y)dpi don't return correct DPI on some devices.
        return dm.densityDpi; 
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

        int dpi = getDpi();

        Log.d(DavaActivity.LOG_TAG, String.format("DavaSurface.surfaceChanged: w=%d, h=%d, surfW=%d, surfH=%d, dpi=%d", w, h, w, h, dpi));
        nativeSurfaceViewOnSurfaceChanged(windowBackendPointer, holder.getSurface(), w, h, w, h, dpi);
        
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
        if (source == InputDevice.SOURCE_MOUSE)
        {
/* uncomment after multidex enabled
            handleMouseEvent(event);
*/
        }
        else if (source == InputDevice.SOURCE_TOUCHSCREEN)
        {
/* uncomment after multidex enabled
            handleTouchEvent(event);
*/
        }
        return true;
    }

    // View.OnKeyListener interface
    @Override
    public boolean onKey(View v, int keyCode, KeyEvent event)
    {
        int source = event.getSource();
        int action = event.getAction();

        // Some SOURCE_GAMEPAD and SOURCE_DPAD events are also SOURCE_KEYBOARD. So first check and process events
        // from gamepad and then from keyboard if not processed.
        if ((source & (InputDevice.SOURCE_GAMEPAD | InputDevice.SOURCE_DPAD)) != 0)
        {
/* uncomment after multidex enabled
            nativeSurfaceViewOnGamepadButton(windowBackendPointer, event.getDeviceId(), action, keyCode);
*/
            return true;
        }

        if ((source & InputDevice.SOURCE_KEYBOARD) == InputDevice.SOURCE_KEYBOARD)
        {
/* uncomment after multidex enabled
            int modifierKeys = event.getMetaState();
            int unicodeChar = event.getUnicodeChar();
            boolean isRepeated = event.getRepeatCount() > 0;
            nativeSurfaceViewOnKeyEvent(windowBackendPointer, action, keyCode, unicodeChar, modifierKeys, isRepeated);
*/
            return true;
        }
        return false;
    }

/* uncomment after multidex enabled
    // View.OnGenericMotionListener interface
    @Override
    public boolean onGenericMotion(View v, MotionEvent event)
    {
        int source = event.getSource();
        if (source == InputDevice.SOURCE_GAMEPAD || source == InputDevice.SOURCE_DPAD || source == InputDevice.SOURCE_JOYSTICK)
        {
            handleGamepadMotionEvent(event);
            return true;
        }
        else if (source == InputDevice.SOURCE_MOUSE)
        {
            handleMouseEvent(event);
            return true;
        }
        return false;
    }
*/

/* uncomment after multidex enabled
    private void handleGamepadMotionEvent(MotionEvent event)
    {
        int action = event.getActionMasked();
        if (action == MotionEvent.ACTION_MOVE)
        {
            int deviceId = event.getDeviceId();
            // First check whether gamepad is in list of attached devices
            DavaGamepadManager.Gamepad gamepad = DavaActivity.gamepadManager().getGamepad(deviceId);
            if (gamepad != null)
            {
                int actionIndex = event.getActionIndex();
                for (int i = 0, n = gamepad.axes.size();i < n;++i)
                {
                    InputDevice.MotionRange r = gamepad.axes.get(i);
                    int axis = r.getAxis();
                    float value = event.getAxisValue(axis, actionIndex);
                    // Notify native code only if axis value has changed
                    // For now simply compare floats (alternatives are floatToIntBits or java.lang.Float.compare)
                    if (gamepad.axisValues[i] != value)
                    {
                        gamepad.axisValues[i] = value;
                        nativeSurfaceViewOnGamepadMotion(windowBackendPointer, deviceId, axis, value);
                    }
                }
            }
        }
    }

    private void handleTouchEvent(MotionEvent event)
    {
        int action = event.getActionMasked();
        int pointerCount = event.getPointerCount();
        int modifierKeys = event.getMetaState();
        switch (action)
        {
        case MotionEvent.ACTION_MOVE:
            for (int i = 0;i < pointerCount;++i)
            {
                int pointerId = event.getPointerId(i);
                float x = event.getX(i);
                float y = event.getY(i);
                nativeSurfaceViewOnTouchEvent(windowBackendPointer, MotionEvent.ACTION_MOVE, pointerId, x, y, modifierKeys);
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
            nativeSurfaceViewOnTouchEvent(windowBackendPointer, action, pointerId, x, y, modifierKeys);
            break;
        }
        case MotionEvent.ACTION_CANCEL:
            for (int i = 0;i < pointerCount;++i)
            {
                int pointerId = event.getPointerId(i);
                float x = event.getX(i);
                float y = event.getY(i);
                nativeSurfaceViewOnTouchEvent(windowBackendPointer, MotionEvent.ACTION_UP, pointerId, x, y, modifierKeys);
            }
            break;
        default:
            break;
        }
    }

    private void handleMouseEvent(MotionEvent event)
    {
        int action = event.getActionMasked();
        int buttonState = event.getButtonState();
        float x = event.getX(0);
        float y = event.getY(0);
        float deltaX = event.getAxisValue(MotionEvent.AXIS_HSCROLL, 0);
        float deltaY = event.getAxisValue(MotionEvent.AXIS_VSCROLL, 0);
        int modifierKeys = event.getMetaState();
        nativeSurfaceViewOnMouseEvent(windowBackendPointer, action, buttonState, x, y, deltaX, deltaY, modifierKeys);
    }
*/
}
