package com.dava.framework;
import java.util.LinkedList;
import java.util.List;

import android.graphics.Rect;
import android.view.View;
import android.view.ViewTreeObserver;

public class SoftKeyboardStateHelper implements ViewTreeObserver.OnGlobalLayoutListener {

    public interface SoftKeyboardStateListener {
        void onSoftKeyboardOpened(Rect keyboardRect);
        void onSoftKeyboardClosed();
    }

    private final List<SoftKeyboardStateListener> listeners = new LinkedList<SoftKeyboardStateListener>();
    private final View activityRootView;
    private Rect       lastSoftKeyboardBounds;
    private boolean    isSoftKeyboardOpened;

    public SoftKeyboardStateHelper(View activityRootView) {
        this(activityRootView, false);
    }

    public SoftKeyboardStateHelper(View activityRootView, boolean isSoftKeyboardOpened) {
        this.activityRootView     = activityRootView;
        this.isSoftKeyboardOpened = isSoftKeyboardOpened;
        activityRootView.getViewTreeObserver().addOnGlobalLayoutListener(this);
    }

    @Override
    public void onGlobalLayout() {
        final int KEYBOARD_DETECTION_HEIGHT = 50;
        Rect r = new Rect();
        activityRootView.getWindowVisibleDisplayFrame(r);
        final int heightDiff = activityRootView.getRootView().getHeight() - (r.bottom - r.top);
        
        if (!isSoftKeyboardOpened && heightDiff > KEYBOARD_DETECTION_HEIGHT) {
            isSoftKeyboardOpened = true;
            notifyOnSoftKeyboardOpened(new Rect(r.left, r.bottom, r.right, r.bottom + heightDiff));
        } else if (isSoftKeyboardOpened && heightDiff < KEYBOARD_DETECTION_HEIGHT) {
            isSoftKeyboardOpened = false;
            notifyOnSoftKeyboardClosed();
        }
    }

    public void setIsSoftKeyboardOpened(boolean isSoftKeyboardOpened) {
        this.isSoftKeyboardOpened = isSoftKeyboardOpened;
    }

    public boolean isSoftKeyboardOpened() {
        return isSoftKeyboardOpened;
    }

    public Rect getLastSoftKeyboardBounds() {
        return lastSoftKeyboardBounds;
    }

    public void addSoftKeyboardStateListener(SoftKeyboardStateListener listener) {
        listeners.add(listener);
    }

    public void removeSoftKeyboardStateListener(SoftKeyboardStateListener listener) {
        listeners.remove(listener);
    }

    private void notifyOnSoftKeyboardOpened(Rect rect) {
        this.lastSoftKeyboardBounds = rect;
        for (SoftKeyboardStateListener listener : listeners) {
            if (listener != null) {
                listener.onSoftKeyboardOpened(rect);
            }
        }
    }

    private void notifyOnSoftKeyboardClosed() {
        for (SoftKeyboardStateListener listener : listeners) {
            if (listener != null) {
                listener.onSoftKeyboardClosed();
            }
        }
    }
}