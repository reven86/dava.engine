package com.dava.engine;

import android.graphics.Rect;
import android.graphics.PixelFormat;
import android.view.Gravity;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.widget.FrameLayout;
import java.util.LinkedList;
import java.util.List;

public class DavaKeyboardState implements ViewTreeObserver.OnGlobalLayoutListener
{
    public interface KeyboardStateListener
    {
        void onKeyboardOpened(Rect keyboardRect);
        void onKeyboardClosed();
    }

    private View contentView = null;
    private FrameLayout layout = null;
    private boolean isKeyboardOpen = false;
    private Rect keyboardRect = new Rect();
    private List<KeyboardStateListener> listeners = new LinkedList<KeyboardStateListener>();

    void start()
    {
        if (layout == null)
        {
            contentView = DavaActivity.instance().findViewById(android.R.id.content);
            WindowManager.LayoutParams params = new WindowManager.LayoutParams(
                        0,
                        WindowManager.LayoutParams.MATCH_PARENT,
                        WindowManager.LayoutParams.TYPE_APPLICATION_PANEL,
                        WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE
                                | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                                | WindowManager.LayoutParams.FLAG_FULLSCREEN
                                | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN,
                        PixelFormat.TRANSPARENT);
            params.softInputMode = WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE;
            params.packageName = DavaActivity.instance().getApplication().getPackageName();
            params.gravity = Gravity.LEFT | Gravity.TOP;
            params.token = contentView.getWindowToken();

            layout = new FrameLayout(DavaActivity.instance());
            DavaActivity.instance().getWindowManager().addView(layout, params);

            layout.getViewTreeObserver().addOnGlobalLayoutListener(this);
        }
    }

    void stop()
    {
        if (layout != null)
        {
            layout.getViewTreeObserver().removeOnGlobalLayoutListener(this);
            DavaActivity.instance().getWindowManager().removeView(layout);

            keyboardRect.left = 0;
            keyboardRect.top = 0;
            keyboardRect.right = 0;
            keyboardRect.bottom = 0;
            isKeyboardOpen = false;

            layout = null;
            contentView = null;

            emitKeyboardClosed();
        }
    }

    public Rect keyboardRect()
    {
        return keyboardRect;
    }

    public boolean isKeyboardOpen()
    {
        return isKeyboardOpen;
    }

    public void addKeyboardStateListener(KeyboardStateListener l)
    {
        if (l != null)
        {
            listeners.add(l);
        }
    }

    public void removeKeyboardStateListener(KeyboardStateListener l)
    {
        listeners.remove(l);
    }

    @Override
    public void onGlobalLayout()
    {
        Rect rc = new Rect();
        layout.getWindowVisibleDisplayFrame(rc);

        int viewHeight = layout.getRootView().getHeight();
        int heightThreshold = viewHeight / 4;  
        int dy = viewHeight - rc.height();

        if (dy > heightThreshold)
        {
            if (!isKeyboardOpen)
            {
                keyboardRect.left = rc.left;
                keyboardRect.top = rc.bottom;
                keyboardRect.right = rc.right;
                keyboardRect.bottom = rc.bottom + dy;

                isKeyboardOpen = true;
                emitKeyboardOpened();
            }
        }
        else
        {
            if (isKeyboardOpen)
            {
                emitKeyboardClosed();
                isKeyboardOpen = false;
            }
        }
    }

    private void emitKeyboardOpened()
    {
        for (KeyboardStateListener l : listeners)
        {
            l.onKeyboardOpened(keyboardRect);
        }
    }

    private void emitKeyboardClosed()
    {
        for (KeyboardStateListener l : listeners)
        {
            l.onKeyboardClosed();
        }
    }
}
