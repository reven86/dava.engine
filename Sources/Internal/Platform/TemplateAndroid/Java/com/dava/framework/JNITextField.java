package com.dava.framework;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

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

    private static final int TEXT_FIELD_HIDE_FROM_SCREEN_STEP = 20000;
    private static final int TEXT_CHANGE_DELAY_REFRESH = 100;
    private static final int NO_ACTIVE_TEXTFIELD = -1;
    // Sum of required IME options for set to view
    private static final int STABLE_IME_OPTIONS = EditorInfo.IME_FLAG_NO_FULLSCREEN;
    private static final int CLOSE_KEYBOARD_DELAY = 30;
    private static final String TAG = "JNITextField";

    private static volatile int activeTextField = NO_ACTIVE_TEXTFIELD;
    private static volatile int lastClosedTextField = NO_ACTIVE_TEXTFIELD;
    private static volatile boolean readyToClose = false;
    private static SoftKeyboardStateHelper keyboardHelper = null;
    private static AttachedFrameLayout keyboardLayout = null;
    private static Handler handler = new Handler();
    private static int lastSelectedImeMode = 0;
    private static int lastSelectedInputType = 0;
    private static final InputFilter[] emptyFilterArray = new InputFilter[0];
    
    static class ControlNotFoundException extends RuntimeException
    {
        public ControlNotFoundException(String string) {
            super(string);
        }

        private static final long serialVersionUID = 5293533328872853457L;
    }
    
    static abstract class SafeRunnable implements Runnable
    {

        @Override
        public void run() {
            try
            {
                safeRun();
            }catch(ControlNotFoundException ex)
            {
                Log.e(TAG, "can't found TextField:\n" + ex.getMessage());
            }
        }
        
        public abstract void safeRun();
    }
    
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
            if(!JNIGLSurfaceView.isPaused())
            {
                TextFieldUpdateTexture(id, pixels, width, height);
            }
            else
            {
                String name = Thread.currentThread().getName();
                Log.e(TAG, "can't update static texture\n"
                        + "for TextField(" + id +")\n"
                        +"GLSurfaceView is paused\n"
                        + "current thread name:"+name);
            }
        }
    }
    
    static class TextField extends EditText
    {
        private final int id;
        private boolean logicVisible = false; 
        
        private volatile boolean isRenderToTexture = false;
        
        private int pixels[] = null;
        private int width = 0;
        private int height = 0;
        
        private int viewWidth;
        private int viewHeight;
        
//        private CastomTextWatcher textWatcher = null;
        private InputFilter[] textFilters = null;
        private String lastStringFromCPP = null;
        
        TextField(int id, Context ctx, int startWidth, int startHeight)
        {
            super(ctx);
            this.id = id;
            viewWidth = startWidth;
            viewHeight = startHeight;
        }
        
        public void setTextFromCPP(String str)
        {
            lastStringFromCPP = str;
        }
        
        public String getTextFromCPP()
        {
            return lastStringFromCPP;
        }
        
        public void setRenderToTexture(boolean value)
        {
            isRenderToTexture = value;
            
            FrameLayout.LayoutParams params = (FrameLayout.LayoutParams)getLayoutParams();
            if (isRenderToTexture)
            {
                if (params.leftMargin < JNITextField.TEXT_FIELD_HIDE_FROM_SCREEN_STEP)
                {
                    params.leftMargin += JNITextField.TEXT_FIELD_HIDE_FROM_SCREEN_STEP;
                }
            } else
            {
                if (params.leftMargin >= JNITextField.TEXT_FIELD_HIDE_FROM_SCREEN_STEP)
                {
                    params.leftMargin -= JNITextField.TEXT_FIELD_HIDE_FROM_SCREEN_STEP;
                }
            }
            setLayoutParams(params);

            updateStaticTexture();
        }

        private void clearStaticTexture() {
            // clear static texture
            JNIActivity activity = JNIActivity.GetActivity();
            UpdateTexture task = new UpdateTexture(id, null, 0, 0);
            activity.PostEventToGL(task);
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

        private void renderToTexture() {
            // if we do not create bitmap do not recycle it
            boolean destroyBitmap = false;
            
            buildDrawingCache();
            Bitmap bitmap = getDrawingCache(); //renderToBitmap();
            if (bitmap == null) // could be if onDraw not called yet
            {
                if (viewHeight <= 0 || viewHeight <= 0)
                {
                    // TextField not fully constructed yet
                    return;
                }
                int specWidth = MeasureSpec.makeMeasureSpec(viewWidth, MeasureSpec.EXACTLY);
                int specHeight = MeasureSpec.makeMeasureSpec(viewHeight, MeasureSpec.EXACTLY);
                measure(specWidth, specHeight);
                int measuredWidth = getMeasuredWidth();
                int measuredHeight = getMeasuredHeight();
                if (measuredHeight <= 0 || measuredWidth <= 0)
                {
                    Log.e(TAG, "can't measure layout for TextField with id:"
                            + id + " w = " + viewWidth + " h = " + viewHeight);
                    return;
                }
                bitmap = Bitmap.createBitmap(measuredWidth, measuredHeight, Bitmap.Config.ARGB_8888);
                Canvas c = new Canvas(bitmap);
                layout(0, 0, measuredWidth, measuredHeight);
                draw(c);

                destroyBitmap = true;
            }

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
                
                if (destroyBitmap)
                {
                    bitmap.recycle();
                }
            }
            JNIActivity activity = JNIActivity.GetActivity();
            UpdateTexture task = new UpdateTexture(id, pixels, width, height);
            activity.PostEventToGL(task);
        }
        
        public void setVisible(boolean value) {
            // remember visibility to restore after unlock screen
            logicVisible = value;
            
            if (logicVisible)
            {
                setVisibility(View.VISIBLE);
            }
            else
            {
                setVisibility(View.GONE);
            }
        }
        
        public boolean isVisible()
        {
            return logicVisible;
        }
        
        public void restoreVisibility()
        {
            setVisible(logicVisible);
        }
        
        public void updateStaticTexture()
        {
            // Workaround if text empty but image cache
            // return previous image and set it back to static
            // texture in native control
            if (0 == getText().length())
            {
                clearStaticTexture();
            }
            else
            {
                if (isRenderToTexture)
                {
                    renderToTexture();
                }
                else
                {
                    clearStaticTexture();
                }
            }
        }
        
//        public void setCastomTextWatcher(CastomTextWatcher t)
//        {
//            textWatcher = t;
//        }
        
//        public void disableTextChangeListener()
//        {
//            removeTextChangedListener(textWatcher);
//            super.setFilters(emptyFilterArray);
//        }
//        
//        public void enableTextChangeListener()
//        {
//            super.setFilters(textFilters);
//            addTextChangedListener(textWatcher);
//        }
        
        @Override
        public void setFilters(InputFilter[] f)
        {
            textFilters = f;
            super.setFilters(textFilters);
        }
    }

    static Map<Integer, TextField> textFields = new HashMap<Integer, TextField>();

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
        TextField tf = textFields.get(id);
        if (null == tf) {
            Log.e(TAG, String.format("Unknown control id:%d", id));
            // better crush inside android VM then in c++ with SIGSEGV
            throw new ControlNotFoundException("can't find JNITextField by id:"+id);
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
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                JNIActivity activity = JNIActivity.GetActivity();
                if (null == activity || activity.GetIsPausing())
                    return;

                if (textFields.containsKey(id)) {
                    Log.e(TAG, String.format("Control with id:%d already created", id));
                    return;
                }
                int viewWidth = Math.round(dx);
                int viewHeight = Math.round(dy);
                
                final TextField text = new TextField(id, activity, viewWidth, viewHeight);

                FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
                        viewWidth, viewHeight);
                params.leftMargin = Math.round(x);
                params.topMargin = Math.round(y);
                params.gravity = Gravity.LEFT | Gravity.TOP;
                text.setPadding(0, 0, 0, 0);
                text.setSingleLine(true);
                text.setBackgroundColor(Color.TRANSPARENT);
                text.setTextColor(Color.WHITE);
                text.setVisibility(View.GONE);
                text.setImeOptions(STABLE_IME_OPTIONS);
                // for static render to texture
                text.setDrawingCacheEnabled(true);

                activity.addContentView(text, params);

                InputFilter inputFilter = new InputFilter() {
                    private final int _id = id;

                    @Override
                    public CharSequence filter(CharSequence source,
                            final int start, final int end, Spanned dest,
                            final int dstart, final int dend) {

                        // Avoiding the line breaks in the single-line text
                        // fields. Line breaks should be replaced with spaces.
                        TextField textField = GetTextField(_id);
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
                        return source;
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
                        JNIActivity.GetActivity().PostEventToGL(new SafeRunnable() {
                            @Override
                            public void safeRun() {
                                JNITextField
                                        .TextFieldFocusChanged(id, hasFocus);
                                final TextField text = GetTextField(id);
                                text.updateStaticTexture();
                                
                                // Workaround we have to call one more time
                                // updateStaticTexture with delay
                                // because if control is password 
                                // android can visually convert ****1 to *****
                                // with some delay
                                Runnable runnable = new Runnable(){
                                    @Override
                                    public void run() {
                                        if(JNIActivity.GetActivity().GetIsPausing())
                                        {
                                            return;
                                        }
                                        text.updateStaticTexture();
                                    }
                                };
                                
                                handler.postDelayed(runnable, JNITextField.TEXT_CHANGE_DELAY_REFRESH);
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

                TextWatcher textWatcher = new TextWatcher() {
                    private String oldText = "";
                    private byte[] emptyArray = new byte[0];

                    @Override
                    public void onTextChanged(final CharSequence s, int start,
                            int before, int count) {
                        final String newValue = s.toString();
                        // if text had been set from c++ skip 
                        // call back
                        Runnable action = new Runnable(){
                            @Override
                            public void run() {
                                String textFromCPP = text.getTextFromCPP();
                                if(textFromCPP == null 
                                   || !textFromCPP.equals(newValue))
                                {
                                    text.setTextFromCPP(newValue);
                                    byte[] newBytes = emptyArray;
                                    byte[] oldBytes = emptyArray;
                                    try {
                                        newBytes = s.toString().getBytes("UTF-8");
                                        oldBytes = oldText.getBytes("UTF-8");
                                    } catch (UnsupportedEncodingException e) {
                                        Log.e(JNIConst.LOG_TAG, e.getMessage());
                                    }
                                    TextFieldKeyPressed(
                                            id, 0, oldBytes.length, newBytes);
                                    TextFieldOnTextChanged(id, newBytes, oldBytes);
                                }
                            }
                        };
                        JNIActivity.GetActivity().PostEventToGL(action);
                    }

                    @Override
                    public void beforeTextChanged(CharSequence s, int start,
                            int count, int after) {
                        oldText = (s == null) ? "" : s.toString();
                    }

                    @Override
                    public void afterTextChanged(Editable s) {
                        
                    }
                };
                
                TextWatcher updateTexture = new TextWatcher() {
                    @Override
                    public void onTextChanged(CharSequence s, int start,
                            int before, int count) {
                    }

                    @Override
                    public void beforeTextChanged(CharSequence s, int start,
                            int count, int after) {
                    }

                    @Override
                    public void afterTextChanged(Editable s) {
                        Runnable runnable = new Runnable(){
                            @Override
                            public void run() {
                                if(JNIActivity.GetActivity().GetIsPausing())
                                {
                                    return;
                                }
                                text.updateStaticTexture();
                            }
                        };
                        // first call update static ASAP
                        handler.post(runnable);
                        // second call it with delay for 
                        // fix some incorrect old text from cache
                        handler.postDelayed(runnable, JNITextField.TEXT_CHANGE_DELAY_REFRESH);
                    }
                };
                text.addTextChangedListener(updateTexture);
                text.addTextChangedListener(textWatcher);
                //text.setCastomTextWatcher(textWatcher);
                

                textFields.put(id, text);
            }
        });
    }

    static void Destroy(final int id) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                final EditText editText = GetTextField(id);
                editText.clearFocus(); // Clear focus before destroying to try
                                       // to close keyboard
                ViewGroup parent = (ViewGroup) editText.getParent();
                if (parent != null)
                    parent.removeView(editText);
                textFields.remove(id);
            }
        });
    }

    public static void UpdateRect(final int id, final float x, final float y,
            final float dx, final float dy) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                TextField editText = GetTextField(id);
                
                FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) editText
                        .getLayoutParams();
                params.leftMargin = Math.round(x);

                int xRoundPos = (int)(x + 0.5f);

                if (editText.isRenderToTexture())
                {
                    params.leftMargin = xRoundPos + JNITextField.TEXT_FIELD_HIDE_FROM_SCREEN_STEP;
                } else
                {
                    params.leftMargin = xRoundPos;
                }
                
                params.topMargin = Math.round(y);
                params.width = Math.round(dx);
                params.height = Math.round(dy);
                
                editText.viewWidth = params.width;
                editText.viewHeight = params.height;
                
                editText.setLayoutParams(params);
            }
        });
    }

    public static void SetText(final int id, final String string) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                if(JNIActivity.GetActivity().GetIsPausing())
                {
                    return;
                }
                final TextField text = GetTextField(id);
                text.setTextFromCPP(string);

                text.setText(string);
                
                text.post(new Runnable() {
                    @Override
                    public void run() {
                        text.updateStaticTexture();
                    }
                });
            }
        });
    }

    public static void SetTextColor(final int id, final float r, final float g,
            final float b, final float a) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                if(JNIActivity.GetActivity().GetIsPausing())
                {
                    return;
                }
                final TextField text = GetTextField(id);
                text.setTextColor(Color.argb((int) (255 * a), (int) (255 * r),
                        (int) (255 * g), (int) (255 * b)));
                text.updateStaticTexture();
            }
        });
    }

    public static void SetFontSize(final int id, final float size) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                if(JNIActivity.GetActivity().GetIsPausing())
                {
                    return;
                }
                final TextField text = GetTextField(id);
                text.setTextSize(TypedValue.COMPLEX_UNIT_PX, (int) size);
                text.updateStaticTexture();
            }
        });
    }

    public static void SetIsPassword(final int id, final boolean isPassword) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                if(JNIActivity.GetActivity().GetIsPausing())
                {
                    return;
                }
                final TextField text = GetTextField(id);
                // Workaround android on change edit type will internally
                // call setText and listeners will trigger this is not what
                // we need
                //text.disableTextChangeListener();
                
                if (isPassword) {
                    text.setInputType(EditorInfo.TYPE_CLASS_TEXT
                            | EditorInfo.TYPE_TEXT_VARIATION_PASSWORD);
                } else {
                    text.setInputType(text.getInputType()
                            & ~(EditorInfo.TYPE_TEXT_VARIATION_PASSWORD));
                }
                
                text.post(new Runnable(){
                    @Override
                    public void run() {
                        text.updateStaticTexture();
                    }
                });
            }
        });
    }

    public static void SetTextUseRtlAlign(final int id, final boolean useRtlAlign) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                if(JNIActivity.GetActivity().GetIsPausing())
                {
                    return;
                }
                TextField text = GetTextField(id);
                int gravity = text.getGravity();
                if (useRtlAlign) {
                    text.setGravity(gravity | Gravity.RELATIVE_LAYOUT_DIRECTION);
                } else {
                    text.setGravity(gravity
                            & ~Gravity.RELATIVE_LAYOUT_DIRECTION);
                }
                text.updateStaticTexture();
            }
        });
    }

    public static void SetTextAlign(final int id, final int align) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                if(JNIActivity.GetActivity().GetIsPausing())
                {
                    return;
                }
                TextField text = GetTextField(id);
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
                text.updateStaticTexture();
            }
        });
    }

    public static void SetInputEnabled(final int id, final boolean value) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                TextField text = GetTextField(id);
                text.setEnabled(value);
            }
        });
    }

    public static void SetAutoCapitalizationType(final int id,
            final int autoCapitalizationType) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                final EditText text = GetTextField(id);
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

    public static void SetAutoCorrectionType(final int id,
            final int autoCorrectionType) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                TextField text = GetTextField(id);
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
                text.setInputType(autoCorrectionFlag);
            }
        });
    }

    public static void SetSpellCheckingType(final int id, final int spellCheckingType) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                final EditText text = GetTextField(id);
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

    public static void SetKeyboardType(final int id, final int keyboardType) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                final EditText text = GetTextField(id);
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

    public static void SetReturnKeyType(final int id, final int returnKeyType) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                final EditText text = GetTextField(id);
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
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
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
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                final EditText text = GetTextField(id);
                if(!text.hasFocus())
                {
                    boolean result = text.requestFocus();
                    if (result == false) {
                        Log.e(TAG, "OpenKeyboard can't took focus");
                    }
                }
            }
        });
    }

    public static void CloseKeyboard(final int id) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                final EditText text = GetTextField(id);
                if(text.hasFocus())
                {
                    text.clearFocus();
                }
            }
        });
    }

    public static void SetEnableReturnKeyAutomatically(int id, boolean value) {
        Log.e(TAG, "SetEnableReturnKeyAutomatically not supported on android");
    }

    public static void SetKeyboardAppearanceType(int id, int value) {
        Log.e(TAG, "SetKeyboardAppearanceType not supported on android");
    }

    public static int GetCursorPos(int id) {
        final EditText text = GetTextField(id);
        int pos = text.getSelectionStart();
        return pos;
    }

    public static void SetCursorPos(final int id, final int pos) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                final EditText text = GetTextField(id);
                text.setSelection(pos);
            }
        });
    }

    public static void SetMaxLength(final int id, final int maxLength) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                final TextField nativeEditText = GetTextField(id);
                
                removeMaxLengthFilters(nativeEditText);
                
                if (maxLength > 0)
                {
                    addOneMoreFilterToArray(maxLength, nativeEditText);
                }
            }

            private void addOneMoreFilterToArray(final int maxLength,
                    final TextField nativeEditText) {
                InputFilter[] filters = nativeEditText.getFilters();
                InputFilter[] newFilters = new InputFilter[filters.length + 1];
                System.arraycopy(filters, 0, newFilters, 0, filters.length);
                newFilters[filters.length] = new InputFilter.LengthFilter(maxLength);
                nativeEditText.setFilters(newFilters);
            }

            private void removeMaxLengthFilters(final TextField nativeEditText) {
                InputFilter[] filters = nativeEditText.getFilters();

                boolean foundLengthFilter = false;
                for(InputFilter f : filters)
                {
                    if (f instanceof InputFilter.LengthFilter)
                    {
                        foundLengthFilter = true;
                        break;
                    }
                }

                if (foundLengthFilter)
                {
                    ArrayList<InputFilter> newFilters = new ArrayList<InputFilter>();
                    for(InputFilter f : filters)
                    {
                        if (!(f instanceof InputFilter.LengthFilter))
                        {
                            newFilters.add(f);
                        }
                    }
                    InputFilter[] array = new InputFilter[newFilters.size()];
                    newFilters.toArray(array);
                    nativeEditText.setFilters(array);
                }
            }
        });
    }

    static protected void RelinkNativeControls() {
        final JNIActivity activity = JNIActivity.GetActivity();
        
        for (TextField control : textFields.values()) {
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
        for (TextField textField: textFields.values()) {
            textField.setVisibility(View.GONE);
        }
    }

    public static void ShowVisibleTextFields() {
        JNIActivity.GetActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                for (TextField textField : textFields.values()) {
                    textField.restoreVisibility();
                    textField.updateStaticTexture();
                }
            }
        });
    }
    
    public static void SetRenderToTexture(final int id, final boolean value) {
        JNIActivity.GetActivity().runOnUiThread(new SafeRunnable() {
            @Override
            public void safeRun() {
                if(JNIActivity.GetActivity().GetIsPausing())
                {
                    return;
                }
                final TextField text = GetTextField(id);
                text.setRenderToTexture(false);
            }
        });
    }
    
    public static boolean IsRenderToTexture(final int id) {
        final TextField text = GetTextField(id);
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
