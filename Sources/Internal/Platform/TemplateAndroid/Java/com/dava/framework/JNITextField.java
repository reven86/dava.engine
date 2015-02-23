package com.dava.framework;

import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.os.Handler;
import android.os.IBinder;
import android.text.Editable;
import android.text.InputFilter;
import android.text.InputType;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.AttributeSet;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.TextView;

import com.dava.framework.SoftKeyboardStateHelper.SoftKeyboardStateListener;

public class JNITextField {

    static private final int NO_ACTIVE_TEXTFIELD = -1;
    // Sum of required IME options for set to view
    static private final int STABLE_IME_OPTIONS = EditorInfo.IME_FLAG_NO_FULLSCREEN;
    static private final int CLOSE_KEYBOARD_DELAY = 30;
    static private final String TAG = "JNITextField";

    static volatile int activeTextField = NO_ACTIVE_TEXTFIELD;
    static private volatile int lastClosedTextField = NO_ACTIVE_TEXTFIELD;
    static private volatile boolean readyToClose = false;
    static private SoftKeyboardStateHelper keyboardHelper = null;
    static private AttachedFrameLayout keyboardLayout = null;
    static private Handler handler = new Handler();
    static private int lastSelectedImeMode = 0;
    static private int lastSelectedInputType = 0;
    
    static class UpdateTexture implements Runnable {
        int id;
        int[] pixels;
        int width;
        int height;

        UpdateTexture(int id, int[] pixels, int width, int height) {
            this.id = id;
            this.pixels = pixels;
            this.width = width;
            this.height = height;
        }

        @Override
        public void run() {
            TextFieldUpdateTexture(id, pixels, width, height);
        }
    }
    
    static class TextField extends EditText
    {
        final public int id;
        public InputFilter maxLengthFilter = null;
        boolean visible = false; 
        
        volatile boolean isRenderToTexture = false;
        Bitmap bitmapCache = null;
        
        int pixels[] = null;
        int width = 0;
        int height = 0;
        
        TextField(int id, Context ctx)
        {
            super(ctx);
            this.id = id;
        }
        
        public void setRenderToTexture(boolean value)
        {
            isRenderToTexture = value;
        }
        
        public boolean isRenderToTexture()
        {
            return isRenderToTexture;
        }
        
        @Override
        public boolean onTouchEvent(MotionEvent event) {
            MotionEvent newEvent = MotionEvent.obtain(event);
            newEvent.setLocation(getLeft() + event.getX(),
                    getTop() + event.getY());
            JNIActivity.GetActivity().glView.dispatchTouchEvent(newEvent);
            return super.onTouchEvent(event);
        }

        // Workaround for BACK press when keyboard opened
        @Override
        public boolean onKeyPreIme(int keyCode, KeyEvent event) {
            // Clear focus on BACK key, DON'T close keyboard itself
            if (keyCode == KeyEvent.KEYCODE_BACK) {
                clearFocus();
                return true;
            }
            return super.onKeyPreIme(keyCode, event);
        }
        
        @Override
        public void onDraw(Canvas canvas) {
            super.onDraw(canvas);

            if (isRenderToTexture) {
                Bitmap bitmap = renderToBitmap();

                if (bitmap != null) {
                    if (pixels == null 
                        || width != bitmap.getWidth()
                        || height != bitmap.getHeight()) {
                        width = bitmap.getWidth();
                        height = bitmap.getHeight();
                        pixels = new int[width * height];
                    }
                    // copy ARGB pixels values into our buffer
                    bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
                }
                JNIActivity activity = JNIActivity.GetActivity();
                UpdateTexture task = new UpdateTexture(id, pixels, width, height);
                activity.PostEventToGL(task);
            }
        }
        
        private Bitmap renderToBitmap() {

            if (bitmapCache != null) {
                bitmapCache.recycle();
            }

            // we need to do it every time because this only works with
            // scrolling
            setDrawingCacheEnabled(true);
            buildDrawingCache();
            // Returns an immutable bitmap from the source bitmap.
            // The new bitmap may be the same object as source, or a copy may
            // have been made. It is initialized with the same density as the
            // original bitmap.
            Bitmap cacheImage = getDrawingCache();
            if (cacheImage != null) {
                bitmapCache = Bitmap.createBitmap(cacheImage);
            }

            setDrawingCacheEnabled(false);
            return bitmapCache;
        };
        
        public void setVisible(boolean value) {
            // remember visibility to restore recreate view later
            visible = value;
            
            int nextVisibility = value ? View.VISIBLE : View.INVISIBLE;
            int currentVisibility = getVisibility();

            if (View.VISIBLE == nextVisibility) {
                if (isRenderToTexture) {
                    switch (currentVisibility) {
                    case View.GONE:
                        setVisibility(nextVisibility);
                        break;
                    case View.VISIBLE:
                        setVisibility(View.GONE);
                        setVisibility(nextVisibility);
                        break;
                    case View.INVISIBLE:
                        break;
                    default:
                        break;
                    }
                } else {
                    if (View.INVISIBLE == currentVisibility) {
                        setVisibility(View.GONE);
                    }
                    setVisibility(nextVisibility);
                }
            } else { // INVISIBLE
                if (isRenderToTexture) {
                    switch (currentVisibility) {
                    case View.GONE:
                        setVisibility(View.INVISIBLE);
                        break;
                    case View.VISIBLE:
                        setVisibility(View.GONE);
                        setVisibility(View.INVISIBLE);
                        break;
                    case View.INVISIBLE:
                        break;
                    default:
                        break;
                    }
                } else {
                    setVisibility(View.GONE);
                }
            } // end if VISIBLE
        }
        
        public boolean isVisible()
        {
            return visible;
        }
        
        public void restoreVisibility()
        {
            setVisible(visible);
        }
    }

    static Map<Integer, TextField> controls = new HashMap<Integer, TextField>();

    static class AttachedFrameLayout extends FrameLayout implements
            View.OnAttachStateChangeListener {

        private boolean isAttached = false;

        public AttachedFrameLayout(Context context) {
            super(context);
            addOnAttachStateChangeListener(this);
        }

        public AttachedFrameLayout(Context context, AttributeSet attrs,
                int defStyle) {
            super(context, attrs, defStyle);
            addOnAttachStateChangeListener(this);
        }

        public AttachedFrameLayout(Context context, AttributeSet attrs) {
            super(context, attrs);
            addOnAttachStateChangeListener(this);
        }

        public boolean isAttached() {
            return isAttached;
        }

        @Override
        public void onViewAttachedToWindow(View v) {
            if (v == this) {
                isAttached = true;
            }
        }

        @Override
        public void onViewDetachedFromWindow(View v) {
            if (v == this) {
                isAttached = false;
            }
        }
    }

    private static TextField GetTextField(int id) {
        TextField tf = controls.get(id);
        if (null == tf) {
            Log.e(TAG, String.format("Unknown control id:%d", id));
            throw new RuntimeException("can't find JNITextField by id:"+id);
        }
        return tf;
    }

    public static void InitializeKeyboardLayout(WindowManager manager,
            IBinder windowToken) {
        if (keyboardLayout == null) {
            if (manager == null) {
                Log.e(TAG,
                        "[InitializeKeyboardLayout] WindowManager must be specified");
                return;
            }
            if (windowToken == null) {
                Log.e(TAG,
                        "[InitializeKeyboardLayout] Window token must be specified");
                return;
            }

            // Add new layout to other window with special parameters
            WindowManager.LayoutParams params = new WindowManager.LayoutParams(
                    0, WindowManager.LayoutParams.MATCH_PARENT,
                    WindowManager.LayoutParams.TYPE_APPLICATION_PANEL,
                    WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE
                            | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                            | WindowManager.LayoutParams.FLAG_FULLSCREEN
                            | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN,
                    PixelFormat.TRANSPARENT);
            params.softInputMode = WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE;
            params.packageName = JNIApplication.GetApplication()
                    .getPackageName();
            params.gravity = Gravity.LEFT | Gravity.TOP;
            params.token = windowToken;

            keyboardLayout = new AttachedFrameLayout(JNIActivity.GetActivity());
            manager.addView(keyboardLayout, params);

            // Set UI flags for detect correct size when navigation bar hidden
            JNIActivity.HideNavigationBar(keyboardLayout);
        }

        if (keyboardHelper == null && keyboardLayout != null) {
            // Initialize detecting keyboard height listener
            keyboardHelper = new SoftKeyboardStateHelper(keyboardLayout);
            keyboardHelper
                    .addSoftKeyboardStateListener(new SoftKeyboardStateListener() {
                        @Override
                        public void onSoftKeyboardOpened(final Rect keyboardRect) {
                            // Send open event to native
                            JNIActivity.GetActivity().PostEventToGL(
                                    new Runnable() {
                                        final int localId = activeTextField;

                                        @Override
                                        public void run() {
                                            KeyboardOpened(localId,
                                                    keyboardRect);
                                        }
                                    });
                        }

                        @Override
                        public void onSoftKeyboardClosed() {
                            // Workaround: if keyboard was closed by other IME
                            // type we restore focus
                            if (activeTextField != NO_ACTIVE_TEXTFIELD) {
                                EditText text = GetTextField(activeTextField);
                                if (text != null) {
                                    // Check that we close keyboard w/o going to
                                    // next field with other IME type
                                    if (lastClosedTextField == NO_ACTIVE_TEXTFIELD) {
                                        activeTextField = NO_ACTIVE_TEXTFIELD;
                                        text.clearFocus();
                                    } else {
                                        text.requestFocus();
                                    }
                                }
                            }
                            // Send close event to native
                            JNIActivity.GetActivity().PostEventToGL(
                                    new Runnable() {
                                        final int localId = lastClosedTextField;

                                        @Override
                                        public void run() {
                                            KeyboardClosed(localId);
                                        }
                                    });
                            // Clear IDs of active fields on real close keyboard
                            lastClosedTextField = NO_ACTIVE_TEXTFIELD;
                        }
                    });
        }
    }

    public static void DestroyKeyboardLayout(WindowManager manager) {
        if (manager != null && keyboardLayout != null) {
            try {
                if (keyboardLayout.isAttached()) {
                    manager.removeView(keyboardLayout);
                }
            } catch (IllegalArgumentException ex) {
                // Handle situation when keyboardLayout deleated from manager
                // already
                Log.w(JNIConst.LOG_TAG,
                        "DestroyKeyboardLayout: " + ex.getMessage());
            }
            keyboardLayout = null;
        }
        if (keyboardHelper != null) {
            keyboardHelper.unsubscribe();
            keyboardHelper = null;
        }
        // Workaround: Send keyboard closed event if keyboard helper was
        // destroyed.
        // It happens when activity window lost a focus and keyboard
        // automatically closed.
        if (activeTextField != NO_ACTIVE_TEXTFIELD) {
            // Send event about keyboard closing
            JNIActivity.GetActivity().PostEventToGL(new Runnable() {
                final int localId = activeTextField;

                @Override
                public void run() {
                    KeyboardClosed(localId);
                }
            });
            // Clear focus of text field
            final EditText text = GetTextField(activeTextField);
            if (text != null) {
                text.clearFocus();
            }
            activeTextField = NO_ACTIVE_TEXTFIELD;
        }
        // Workaround: Send close keyboard event if text field lost focus and
        // activity
        // lost focus too before keyboard was hidden (animation not finished)
        else if (lastClosedTextField != NO_ACTIVE_TEXTFIELD) {
            JNIActivity.GetActivity().PostEventToGL(new Runnable() {
                final int localId = lastClosedTextField;

                @Override
                public void run() {
                    KeyboardClosed(localId);
                }
            });
            lastClosedTextField = NO_ACTIVE_TEXTFIELD;
        }
    }

    public static int GetLastKeyboardIMEOptions() {
        return lastSelectedImeMode;
    }

    public static int GetLastKeyboardInputType() {
        return lastSelectedInputType;
    }

    public static void Create(final int id, final float x, final float y,
            final float dx, final float dy) {

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                JNIActivity activity = JNIActivity.GetActivity();
                if (null == activity || activity.GetIsPausing())
                    return;

                if (controls.containsKey(id)) {
                    Log.e(TAG, String.format("Control with id:%d already created", id));
                    return;
                }
                
                final TextField text = new TextField(id, activity);

                FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
                        Math.round(dx), Math.round(dy));
                params.leftMargin = Math.round(x);
                params.topMargin = Math.round(y);
                params.gravity = Gravity.LEFT | Gravity.TOP;
                text.setPadding(0, 0, 0, 0);
                text.setSingleLine(true);
                text.setBackgroundColor(Color.TRANSPARENT);
                text.setTextColor(Color.WHITE);
                text.setVisibility(View.GONE);
                text.setImeOptions(STABLE_IME_OPTIONS);

                activity.addContentView(text, params);

                InputFilter inputFilter = new InputFilter() {
                    private final int _id = id;

                    @Override
                    public CharSequence filter(CharSequence source,
                            final int start, final int end, Spanned dest,
                            final int dstart, final int dend) {

                        // Avoiding the line breaks in the single-line text
                        // fields. Line breaks should be replaced with spaces.
                        EditText textField = GetTextField(_id);
                        if (0 == (textField.getInputType() & (InputType.TYPE_TEXT_FLAG_MULTI_LINE | InputType.TYPE_TEXT_FLAG_IME_MULTI_LINE))) {
                            SpannableStringBuilder s = new SpannableStringBuilder(
                                    source);
                            if (source instanceof Spanned
                                    || source instanceof Spannable) {
                                Spanned spanned = (Spanned) source;
                                TextUtils.copySpansFrom(spanned, start, end,
                                        null, s, 0);
                            }

                            for (int i = 0; i < s.length(); ++i) {
                                if ('\n' == s.charAt(i)) {
                                    s.replace(i, i + 1, " ");
                                }
                            }
                            source = s;
                        }

                        String origSource = source.toString();

                        TextField editText = GetTextField(_id);

                        int sourceRepLen = end - start;
                        int destRepLen = dend - dstart;
                        int curStringLen = editText.getText().length();
                        int newStringLen = curStringLen - destRepLen
                                + sourceRepLen;

                        if (newStringLen >= curStringLen) {
                            if (editText != null
                                    && editText.maxLengthFilter != null) {
                                CharSequence res = editText.maxLengthFilter
                                        .filter(source, start, end, dest,
                                                dstart, dend);
                                if (res != null && res.toString().isEmpty())
                                    return res;
                                if (res != null)
                                    source = res;
                            }
                        }

                        final CharSequence sourceToProcess = source;
                        final String text = editText.getText()
                                .toString();
                        FutureTask<String> t = new FutureTask<String>(
                                new Callable<String>() {
                                    @Override
                                    public String call() throws Exception {
                                        byte[] bytes = sourceToProcess
                                                .toString().getBytes("UTF-8");
                                        int curPos = 0;
                                        int finalStart = dstart;
                                        while (curPos < dstart) {
                                            int codePoint = text
                                                    .codePointAt(curPos);
                                            if (codePoint > 0xFFFF) {
                                                curPos++;
                                                finalStart--;
                                            }
                                            curPos++;
                                        }
                                        byte[] retBytes = TextFieldKeyPressed(
                                                _id, finalStart, dend - dstart,
                                                bytes);
                                        return new String(retBytes, "UTF-8");
                                    }
                                });
                        JNIActivity.GetActivity().PostEventToGL(t);
                        try {
                            String s = t.get();
                            if (s.equals(origSource)) {
                                return null;
                            } else if (s.length() > 0) {
                                return s;
                            }
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        } catch (ExecutionException e) {
                            e.printStackTrace();
                        }

                        return "";
                    }
                };
                text.setFilters(new InputFilter[] { inputFilter });

                text.setOnEditorActionListener(new TextView.OnEditorActionListener() {
                    private final int _id = id;

                    @Override
                    public boolean onEditorAction(TextView v, int actionId,
                            KeyEvent event) {
                        JNIActivity.GetActivity().PostEventToGL(new Runnable() {
                            @Override
                            public void run() {
                                JNITextField.TextFieldShouldReturn(_id);
                            }
                        });
                        return true;
                    }
                });

                text.setOnLongClickListener(new View.OnLongClickListener() {
                    @Override
                    public boolean onLongClick(View v) {
                        return !v.hasFocus();
                    }
                });

                text.setOnFocusChangeListener(new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, final boolean hasFocus) {
                        // Select UITextField when native filed selected (like
                        // iOS)
                        JNIActivity.GetActivity().PostEventToGL(new Runnable() {
                            @Override
                            public void run() {
                                JNITextField
                                        .TextFieldFocusChanged(id, hasFocus);
                            }
                        });

                        // Control keyboard state by changing focus state
                        if (hasFocus) {
                            lastSelectedImeMode = text.getImeOptions();
                            lastSelectedInputType = text.getInputType();

                            activeTextField = id;
                            if (readyToClose) // Another text field lose a focus
                            {
                                // Check that keyboard already shown
                                if (keyboardHelper != null
                                        && keyboardHelper
                                                .isSoftKeyboardOpened()) {
                                    JNIActivity.GetActivity().PostEventToGL(
                                            new Runnable() {
                                                final int localActiveId = activeTextField;
                                                final int localLastCloseId = lastClosedTextField;
                                                final Rect localRect = keyboardHelper
                                                        .getLastSoftKeyboardBounds();

                                                @Override
                                                public void run() {
                                                    // Send close/open events
                                                    // with cached data for
                                                    // simulate iOS behavior
                                                    KeyboardClosed(localLastCloseId);
                                                    KeyboardOpened(
                                                            localActiveId,
                                                            localRect);
                                                }
                                            });
                                }
                                // Cancel physical closing keyboard
                                readyToClose = false;
                            } else // No any focused text fields -> show
                                   // keyboard physically
                            {
                                InputMethodManager imm = (InputMethodManager) JNIActivity
                                        .GetActivity().getSystemService(
                                                Context.INPUT_METHOD_SERVICE);
                                imm.showSoftInput(text,
                                        InputMethodManager.SHOW_IMPLICIT);
                            }
                        } else {
                            lastClosedTextField = id;

                            // Run close keyboard method with delay for cancel
                            // it
                            // if another text field will be selected
                            readyToClose = true;
                            handler.postDelayed(new Runnable() {
                                // Store windowToken if text field will be
                                // detached from window
                                final private IBinder windowToken = text
                                        .getWindowToken();

                                @Override
                                public void run() {
                                    if (readyToClose) // Closing keyboard didn't
                                                      // aborted
                                    {
                                        InputMethodManager imm = (InputMethodManager) JNIActivity
                                                .GetActivity()
                                                .getSystemService(
                                                        Context.INPUT_METHOD_SERVICE);
                                        imm.hideSoftInputFromWindow(
                                                windowToken, 0);
                                        activeTextField = NO_ACTIVE_TEXTFIELD;
                                        readyToClose = false;
                                    }
                                }
                            }, CLOSE_KEYBOARD_DELAY);
                        }
                    }
                });

                text.addTextChangedListener(new TextWatcher() {
                    private String oldText = "";

                    @Override
                    public void onTextChanged(CharSequence s, int start,
                            int before, int count) {
                    }

                    @Override
                    public void beforeTextChanged(CharSequence s, int start,
                            int count, int after) {
                        oldText = (s == null) ? "" : s.toString();
                    }

                    @Override
                    public void afterTextChanged(Editable s) {
                        try {
                            byte[] newBytes = s.toString().getBytes("UTF-8");
                            byte[] oldBytes = oldText.getBytes("UTF-8");
                            TextFieldOnTextChanged(id, newBytes, oldBytes);
                        } catch (UnsupportedEncodingException e) {
                            Log.w(JNIConst.LOG_TAG, e.getMessage());
                            // Send changed text event with two empty strings
                            TextFieldOnTextChanged(id, new byte[] {},
                                    new byte[] {});
                        }
                    }
                });

                controls.put(id, text);
            }
        });
    }

    static void Destroy(final int id) {
        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                final EditText editText = GetTextField(id);
                editText.clearFocus(); // Clear focus before destroying to try
                                       // to close keyboard
                ViewGroup parent = (ViewGroup) editText.getParent();
                if (parent != null)
                    parent.removeView(editText);
                controls.remove(id);
            }
        });
    }

    public static void UpdateRect(final int id, final float x, final float y,
            final float dx, final float dy) {
        final EditText editText = GetTextField(id);
        if (editText == null)
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) editText
                        .getLayoutParams();
                params.leftMargin = Math.round(x);
                params.topMargin = Math.round(y);
                params.width = Math.round(dx);
                params.height = Math.round(dy);
                editText.setLayoutParams(params);
            }
        });
    }

    public static void SetText(int id, final String string) {
        final EditText text = GetTextField(id);
        if (text == null)
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                text.setText(string);
            }
        });
    }

    public static void SetTextColor(int id, final float r, final float g,
            final float b, final float a) {
        final EditText text = GetTextField(id);
        if (text == null)
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                text.setTextColor(Color.argb((int) (255 * a), (int) (255 * r),
                        (int) (255 * g), (int) (255 * b)));
            }
        });
    }

    public static void SetFontSize(int id, final float size) {
        final EditText text = GetTextField(id);
        if (text == null)
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                text.setTextSize(TypedValue.COMPLEX_UNIT_PX, (int) size);
            }
        });
    }

    public static void SetIsPassword(int id, final boolean isPassword) {
        final EditText text = GetTextField(id);
        if (text == null)
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (isPassword) {
                    text.setInputType(EditorInfo.TYPE_CLASS_TEXT
                            | EditorInfo.TYPE_TEXT_VARIATION_PASSWORD);
                } else {
                    text.setInputType(text.getInputType()
                            & ~(EditorInfo.TYPE_TEXT_VARIATION_PASSWORD));
                }
            }
        });
    }

    public static void SetTextUseRtlAlign(int id, final boolean useRtlAlign) {
        final EditText text = GetTextField(id);
        if (text == null)
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                int gravity = text.getGravity();
                if (useRtlAlign) {
                    text.setGravity(gravity | Gravity.RELATIVE_LAYOUT_DIRECTION);
                } else {
                    text.setGravity(gravity
                            & ~Gravity.RELATIVE_LAYOUT_DIRECTION);
                }
            }
        });
    }

    public static void SetTextAlign(int id, final int align) {
        final EditText text = GetTextField(id);
        if (text == null)
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                boolean isRelative = (text.getGravity() & Gravity.RELATIVE_HORIZONTAL_GRAVITY_MASK) > 0;

                int gravityH = Gravity.LEFT;
                int gravityV = Gravity.CENTER_VERTICAL;
                if ((align & 0x01) > 0) // ALIGN_LEFT
                    gravityH = Gravity.LEFT;
                if ((align & 0x02) > 0) // ALIGN_HCENTER
                    gravityH = Gravity.CENTER_HORIZONTAL;
                if ((align & 0x04) > 0) // ALIGN_RIGHT
                    gravityH = Gravity.RIGHT;
                if ((align & 0x08) > 0) // ALIGN_TOP
                    gravityV = Gravity.TOP;
                if ((align & 0x10) > 0) // ALIGN_VCENTER
                    gravityV = Gravity.CENTER_VERTICAL;
                if ((align & 0x20) > 0) // ALIGN_BOTTOM
                    gravityV = Gravity.BOTTOM;

                if (isRelative) {
                    gravityH |= Gravity.RELATIVE_LAYOUT_DIRECTION;
                }
                text.setGravity(gravityH | gravityV);
            }
        });
    }

    public static void SetInputEnabled(int id, final boolean value) {
        final EditText text = GetTextField(id);
        if (text == null)
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                text.setEnabled(value);
            }
        });
    }

    public static void SetAutoCapitalizationType(int id,
            final int autoCapitalizationType) {
        final EditText text = GetTextField(id);
        if (text == null)
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                int autoCapitalizationFlag = text.getInputType();
                autoCapitalizationFlag &= ~(InputType.TYPE_TEXT_FLAG_CAP_WORDS
                        | InputType.TYPE_TEXT_FLAG_CAP_SENTENCES | InputType.TYPE_TEXT_FLAG_CAP_CHARACTERS);
                switch (autoCapitalizationType) {
                case 1: // AUTO_CAPITALIZATION_TYPE_WORDS
                    autoCapitalizationFlag |= InputType.TYPE_TEXT_FLAG_CAP_WORDS;
                    break;
                case 2: // AUTO_CAPITALIZATION_TYPE_SENTENCES
                    autoCapitalizationFlag |= InputType.TYPE_TEXT_FLAG_CAP_SENTENCES;
                    break;
                case 3: // AUTO_CAPITALIZATION_TYPE_ALL_CHARS
                    autoCapitalizationFlag |= InputType.TYPE_TEXT_FLAG_CAP_CHARACTERS;
                    break;
                case 0: // AUTO_CAPITALIZATION_TYPE_NONE
                default:
                    break;
                }
                text.setInputType(autoCapitalizationFlag);
            }
        });
    }

    public static void SetAutoCorrectionType(int id,
            final int autoCorrectionType) {
        final EditText text = GetTextField(id);
        if (text == null)
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                int autoCorrectionFlag = text.getInputType();
                switch (autoCorrectionType) {
                case 0: // AUTO_CORRECTION_TYPE_DEFAULT
                case 2: // AUTO_CORRECTION_TYPE_YES
                    autoCorrectionFlag |= InputType.TYPE_TEXT_FLAG_AUTO_CORRECT;
                    break;
                case 1: // AUTO_CORRECTION_TYPE_NO
                default:
                    autoCorrectionFlag &= ~(InputType.TYPE_TEXT_FLAG_AUTO_CORRECT);
                    break;
                }
                ;
                text.setInputType(autoCorrectionFlag);
            }
        });
    }

    public static void SetSpellCheckingType(int id, final int spellCheckingType) {
        final EditText text = GetTextField(id);
        if (text == null)
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                int spellCheckingFlag = text.getInputType();
                switch (spellCheckingType) {
                case 0: // SPELL_CHECKING_TYPE_DEFAULT
                case 2: // SPELL_CHECKING_TYPE_YES
                    spellCheckingFlag |= InputType.TYPE_TEXT_FLAG_AUTO_COMPLETE;
                    break;
                case 1: // SPELL_CHECKING_TYPE_NO
                default:
                    spellCheckingFlag &= ~(InputType.TYPE_TEXT_FLAG_AUTO_COMPLETE);
                    break;
                }
                text.setInputType(spellCheckingFlag);
            }
        });
    }

    public static void SetKeyboardType(int id, final int keyboardType) {
        final EditText text = GetTextField(id);
        if (text == null)
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {

                int inputFlags = text.getInputType();
                inputFlags &= ~(InputType.TYPE_CLASS_NUMBER
                        | InputType.TYPE_CLASS_TEXT
                        | InputType.TYPE_TEXT_VARIATION_URI
                        | InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS | EditorInfo.TYPE_CLASS_TEXT);

                switch (keyboardType) {
                case 2: // KEYBOARD_TYPE_NUMBERS_AND_PUNCTUATION
                case 4: // KEYBOARD_TYPE_NUMBER_PAD
                case 5: // KEYBOARD_TYPE_PHONE_PAD
                case 6: // KEYBOARD_TYPE_NAME_PHONE_PAD
                case 8: // KEYBOARD_TYPE_DECIMAL_PAD
                    inputFlags |= InputType.TYPE_CLASS_NUMBER;
                    break;

                case 3: // KEYBOARD_TYPE_URL
                case 9: // KEYBOARD_TYPE_TWITTER
                    inputFlags |= InputType.TYPE_CLASS_TEXT
                            | InputType.TYPE_TEXT_VARIATION_URI;
                    break;

                case 7: // KEYBOARD_TYPE_EMAIL_ADDRESS
                    inputFlags |= InputType.TYPE_CLASS_TEXT
                            | InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS;
                    break;

                case 0: // KEYBOARD_TYPE_DEFAULT
                case 1: // KEYBOARD_TYPE_ASCII_CAPABLE
                default:
                    inputFlags |= EditorInfo.TYPE_CLASS_TEXT;
                    break;
                }
                text.setInputType(inputFlags);
            }
        });
    }

    public static void SetReturnKeyType(int id, final int returnKeyType) {
        final EditText text = GetTextField(id);
        if (text == null)
            return;
        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                int imeOptions = 0;// EditorInfo.IME_ACTION_DONE;
                switch (returnKeyType) {
                case 1: // RETURN_KEY_GO,
                    imeOptions = EditorInfo.IME_ACTION_GO;
                    break;
                case 2: // RETURN_KEY_GOOGLE,
                case 6: // RETURN_KEY_SEARCH,
                case 8: // RETURN_KEY_YAHOO,
                    imeOptions = EditorInfo.IME_ACTION_SEARCH;
                    break;
                case 4: // RETURN_KEY_NEXT,
                    imeOptions = EditorInfo.IME_ACTION_NEXT;
                    break;
                case 7: // RETURN_KEY_SEND,
                    imeOptions = EditorInfo.IME_ACTION_SEND;
                    break;
                case 0: // RETURN_KEY_DEFAULT = 0,
                case 3: // RETURN_KEY_JOIN,
                case 5: // RETURN_KEY_ROUTE,
                case 9: // RETURN_KEY_DONE,
                case 10: // RETURN_KEY_EMERGENCY_CALL
                default:
                    imeOptions = EditorInfo.IME_ACTION_DONE;
                    break;
                }
                text.setImeOptions(STABLE_IME_OPTIONS | imeOptions);
            }
        });
    }

    public static void SetVisible(final int id, final boolean isVisible) {

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                final TextField textField = GetTextField(id);
                
                if (!isVisible && textField.hasFocus()) {
                    // Clear focus before hiding to try to close keyboard
                    textField.clearFocus(); 
                }
                textField.setVisible(isVisible);
            }
        });
    }

    public static void OpenKeyboard(final int id) {
        final EditText text = GetTextField(id);
        if (text == null || text.hasFocus())
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                boolean result = text.requestFocus();
                if (result == false) {
                    Log.e(TAG, "OpenKeyboard can't took focus");
                }
            }
        });
    }

    public static void CloseKeyboard(int id) {
        final EditText text = GetTextField(id);
        if (text == null || !text.hasFocus())
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                text.clearFocus();
            }
        });
    }

    public static void SetEnableReturnKeyAutomatically(int id, boolean value) {
        Log.d(TAG, "SetEnableReturnKeyAutomatically not supported on android");
    }

    public static void SetKeyboardAppearanceType(int id, int value) {
        Log.d(TAG, "SetKeyboardAppearanceType not supported on android");
    }

    public static int GetCursorPos(int id) {
        final EditText text = GetTextField(id);
        if (text == null)
            return 0;

        int pos = text.getSelectionStart();
        return pos;
    }

    public static void SetCursorPos(int id, final int pos) {
        final EditText text = GetTextField(id);
        if (text == null)
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                text.setSelection(pos);
            }
        });
    }

    public static void SetMaxLength(int id, final int maxLength) {
        final TextField nativeEditText = GetTextField(id);
        if (nativeEditText == null)
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (maxLength > 0) {
                    nativeEditText.maxLengthFilter = new InputFilter.LengthFilter(
                            maxLength);
                } else {
                    nativeEditText.maxLengthFilter = null; // not limited
                }
            }
        });
    }

    static protected void RelinkNativeControls() {
        final JNIActivity activity = JNIActivity.GetActivity();
        
        for (TextField control : controls.values()) {
            ViewGroup viewGroup = (ViewGroup) control.getParent();
            viewGroup.removeView(control);
            activity.addContentView(control, control.getLayoutParams());
        }
    }

    public static void KeyboardOpened(int id, Rect keyboardRect) {
        if (id != NO_ACTIVE_TEXTFIELD) {
            TextFieldKeyboardShown(id, keyboardRect.left, keyboardRect.top,
                    keyboardRect.width(), keyboardRect.height());
        }
    }

    public static void KeyboardClosed(int id) {
        if (id != NO_ACTIVE_TEXTFIELD) {
            JNIActivity.GetActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    JNIActivity activity = JNIActivity.GetActivity();
                    View view = activity.getWindow().getDecorView();
                    JNIActivity.HideNavigationBar(view);
                }

            });
            TextFieldKeyboardHidden(id);
        }
    }

    public static void HideAllTextFields() {
        for (TextField textField: controls.values()) {
            textField.setVisibility(View.GONE);
        }
    }

    public static void ShowVisibleTextFields() {
        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                for (TextField textField : controls.values()) {
                    textField.restoreVisibility();
                }
            }
        });
    }
    
    public static void SetRenderToTexture(int id, boolean value)
    {
        final TextField text = GetTextField(id);
        final boolean needRenderToTexture = value;
        if (text == null)
            return;

        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                text.setRenderToTexture(needRenderToTexture);
            }
        });
    }
    
    public static boolean IsRenderToTexture(int id)
    {
        final TextField text = GetTextField(id);
        if(null == text)
        {
            Log.e(TAG, "not found JNITextField by id == " + id);
            return false;
        }
        return text.isRenderToTexture();
    }

    public static native void TextFieldShouldReturn(int id);

    public static native byte[] TextFieldKeyPressed(int id,
            int replacementLocation, int replacementLength, byte[] byteArray);

    public static native void TextFieldOnTextChanged(int id, byte[] newText,
            byte[] oldText);

    public static native void TextFieldKeyboardShown(int id, int x, int y,
            int dx, int dy);

    public static native void TextFieldKeyboardHidden(int id);

    public static native void TextFieldFocusChanged(int id,
            final boolean hasFocus);
    public static native void TextFieldUpdateTexture(int id, int[] pixels,
            int width, int height);
}
