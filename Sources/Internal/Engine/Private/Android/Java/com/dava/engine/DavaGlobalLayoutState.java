package com.dava.engine;

import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.os.Build;
import android.view.Gravity;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.widget.FrameLayout;

import java.util.LinkedList;
import java.util.List;

public class DavaGlobalLayoutState extends DavaActivity.ActivityListenerImpl implements ViewTreeObserver.OnGlobalLayoutListener {

    public interface GlobalLayoutListener
    {
        void onVisibleFrameChanged(Rect visibleFrame);
    }

    private View contentView = null;
    private FrameLayout layout = null;
    private Rect visibleFrame = new Rect();
    private List<GlobalLayoutListener> listeners = new LinkedList<GlobalLayoutListener>();

    DavaGlobalLayoutState()
    {
        onResume();
    }

    @Override
    public void onResume()
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

    @Override
    public void onPause()
    {
        if (layout != null)
        {
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN)
            {
                layout.getViewTreeObserver().removeGlobalOnLayoutListener(this);
            }
            else
            {
                layout.getViewTreeObserver().removeOnGlobalLayoutListener(this);
            }

            DavaActivity.instance().getWindowManager().removeView(layout);

            visibleFrame.setEmpty();
            layout = null;
            contentView = null;
        }
    }

    public void addGlobalLayoutListener(GlobalLayoutListener l)
    {
        if (l != null)
        {
            listeners.add(l);
        }
    }

    public void removeGlobalLayoutListener(GlobalLayoutListener l)
    {
        listeners.remove(l);
    }

    @Override
    public void onGlobalLayout()
    {
        if (layout != null)
        {
            layout.getWindowVisibleDisplayFrame(visibleFrame);
            emitVisibleFrameChanged();
        }
    }

    private void emitVisibleFrameChanged()
    {
        for (GlobalLayoutListener l : listeners)
        {
            l.onVisibleFrameChanged(visibleFrame);
        }
    }

}
