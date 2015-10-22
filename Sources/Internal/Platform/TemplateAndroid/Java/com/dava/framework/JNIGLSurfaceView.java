package com.dava.framework;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.util.Pair;
import android.view.GestureDetector;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.ViewGroup.LayoutParams;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

import com.bda.controller.ControllerListener;
import com.bda.controller.StateEvent;

public class JNIGLSurfaceView extends GLSurfaceView
{
	private static final int MAX_KEYS = 256; // Maximum number of keycodes which used in native code

	private JNIRenderer mRenderer = null;

	private native void nativeOnInput(int action, int source, int groupSize, ArrayList< InputRunnable.InputEvent > activeInputs, ArrayList< InputRunnable.InputEvent > allInputs);
	private native void nativeOnKeyDown(int keyCode);
	private native void nativeOnKeyUp(int keyCode);
	private native void nativeOnGamepadElement(int elementKey, float value, boolean isKeycode);
	
	private boolean isMultitouchEnabled = true;
	
	private Integer[] gamepadAxises = null;
	private Integer[] overridedGamepadKeys = null;
	private ArrayList< Pair<Integer, Integer> > gamepadButtonsAxisMap = new ArrayList< Pair<Integer, Integer> >();
	
	private static volatile boolean isPaused = false;
	
	MOGAListener mogaListener = null;
	
	boolean[] pressedKeys = new boolean[MAX_KEYS]; // Use MAX_KEYS for mapping keycodes to native

	public int lastDoubleActionIdx = -1;
	
	public static boolean isPaused()
	{
	    return isPaused;
	}
	
	class DoubleTapListener extends GestureDetector.SimpleOnGestureListener{
		JNIGLSurfaceView glview;
		
		DoubleTapListener(JNIGLSurfaceView view) {
			this.glview = view;
		}
		
		@Override
		public boolean onDoubleTap(MotionEvent e) {
			lastDoubleActionIdx = e.getActionIndex();
			
			glview.queueEvent(new InputRunnable(e, 2));
			return true;
		}
	}
	
	GestureDetector doubleTapDetector = null;

	public JNIGLSurfaceView(Context context) 
	{
		super(context);
		Init();
	}

	public JNIGLSurfaceView(Context context, AttributeSet attrs)
	{
		super(context, attrs);
		Init();
	}

	private void Init()
	{
		this.getHolder().setFormat(PixelFormat.TRANSLUCENT);

		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB)
		{
			setPreserveEGLContextOnPause(true);
		}
		setEGLContextFactory(new JNIContextFactory());
		setEGLConfigChooser(new JNIConfigChooser());

		mRenderer = new JNIRenderer();
		setRenderer(mRenderer);
		setRenderMode(RENDERMODE_CONTINUOUSLY);
		
		mogaListener = new MOGAListener(this);
		
		setDebugFlags(0);
		
		doubleTapDetector = new GestureDetector(JNIActivity.GetActivity(), new DoubleTapListener(this));

		gamepadButtonsAxisMap.add(new Pair<Integer, Integer>(KeyEvent.KEYCODE_BUTTON_L2, MotionEvent.AXIS_LTRIGGER));
		gamepadButtonsAxisMap.add(new Pair<Integer, Integer>(KeyEvent.KEYCODE_BUTTON_L2, MotionEvent.AXIS_BRAKE));
		gamepadButtonsAxisMap.add(new Pair<Integer, Integer>(KeyEvent.KEYCODE_BUTTON_R2, MotionEvent.AXIS_RTRIGGER));
		gamepadButtonsAxisMap.add(new Pair<Integer, Integer>(KeyEvent.KEYCODE_BUTTON_R2, MotionEvent.AXIS_GAS));
	}
	
	public void SetAvailableGamepadAxises(Integer[] axises)
	{
		gamepadAxises = axises;
		
		Set<Integer> overridedKeys = new HashSet<Integer>();
		for(Pair<Integer, Integer> p : gamepadButtonsAxisMap) {
			for(Integer a : axises) {
				if(a == p.second) {
					overridedKeys.add(p.first);
				}
			}
		}
		
		overridedGamepadKeys = overridedKeys.toArray(new Integer[overridedKeys.size()]);
	}
	
	public void SetMultitouchEnabled(boolean enabled)
	{
		isMultitouchEnabled = enabled;
	}
	
    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        // Fix lag when text field lost focus, but keyboard not closed yet. 
        outAttrs.imeOptions = JNITextField.GetLastKeyboardIMEOptions();
        outAttrs.inputType = JNITextField.GetLastKeyboardInputType();
        return super.onCreateInputConnection(outAttrs);
    }
    
	@Override
	protected void onSizeChanged(int w, int h, int oldw, int oldh) {
		//YZ rewrite size parameter from fill parent to fixed size
		LayoutParams params = getLayoutParams();
		params.height = h;
		params.width = w;
		super.onSizeChanged(w, h, oldw, oldh);
	}
	
    @Override
    public void onPause() {
        isPaused = true;
        Log.d(JNIConst.LOG_TAG, "Activity JNIGLSurfaceView onPause");
        setRenderMode(RENDERMODE_WHEN_DIRTY);
        queueEvent(new Runnable() {
            public void run() {
                mRenderer.OnPause();
            }
        });
        super.onPause();// destroy eglCondext(or unbind), eglScreen, eglSurface
        // we write AAA mobile game and for user better resume as fast as posible
        // so DO NOT destroy opengl context
    }

    @Override
    public void onResume() {
        Log.d(JNIConst.LOG_TAG, "Activity JNIGLSurfaceView onResume");
        // first call parent to restore eglContext and start gl thread
        super.onResume();
        
        JNIActivity activity = JNIActivity.GetActivity();

        Runnable action = new Runnable() {
            public void run() {
                resumeGameLoop();
            }
        };
        
        if (activity.hasWindowFocus())
        {
            queueEvent(action);
        } else 
        {
            // we have to resume game later on some devices
            // to resolve deadlock
            activity.setResumeGLActionOnWindowReady(action);
        }
        isPaused = false;
    }
    
    private void resumeGameLoop() {
        mRenderer.OnResume();
        setRenderMode(RENDERMODE_CONTINUOUSLY);
    };
	
	public class InputRunnable implements Runnable
	{
		public class InputEvent
		{
			int tid;
			float x;
			float y;
			int tapCount;
			double time;

			InputEvent(int tid, float x, float y, double time)
			{
				this.tid = tid;
				this.x = x;
				this.y = y;
				this.tapCount = 1;
				this.time = time;
			}
			
			InputEvent(int tid, float x, float y, int tapCount, double time)
			{
				this.tid = tid;
				this.x = x;
				this.y = y;
				this.tapCount = tapCount;
				this.time = time;
			}
		}

		ArrayList<InputEvent> activeEvents;
		ArrayList<InputEvent> allEvents;

		int action;
		int source;
		int groupSize;

		int touchIdForPointerId(int pointerId) {
			return pointerId + 1;
		}

		public InputRunnable(final android.view.MotionEvent event, final int tapCount)
		{
			allEvents = new ArrayList<InputEvent>();

			action = event.getActionMasked();
			source = event.getSource();
			groupSize = 1;

			final int historySize = event.getHistorySize();
			final int pointerCount = event.getPointerCount();

			for (int historyStep = 0; historyStep < historySize; historyStep++) {
				for (int i = 0; i < pointerCount; i++) {
					if((source & InputDevice.SOURCE_CLASS_JOYSTICK) > 0) {
						for (int a = 0; a < gamepadAxises.length; ++a) {
							InputEvent ev = new InputEvent(gamepadAxises[a], event.getHistoricalAxisValue(gamepadAxises[a], i, historyStep), 0, tapCount, event.getHistoricalEventTime(historyStep));

							allEvents.add(ev);
						}
					}
				}
			}

			int actionIndex = event.getActionIndex();
			int pointerId = event.getPointerId(actionIndex);
            if ((source & InputDevice.SOURCE_CLASS_POINTER) > 0
                && 
                (android.view.MotionEvent.ACTION_MOVE == action
                || android.view.MotionEvent.ACTION_CANCEL == action)) {

                groupSize = pointerCount;
                
                for (int i = 0; i < pointerCount; i++) {
                    pointerId = event.getPointerId(i);
                    if (isMultitouchEnabled) {
                        InputEvent ev = new InputEvent(
                                touchIdForPointerId(pointerId), event.getX(i),
                                event.getY(i), tapCount, event.getEventTime());
                        allEvents.add(ev);
                    }
                }
            } else
            {
                InputEvent ev = new InputEvent(
                        touchIdForPointerId(pointerId),
                        event.getX(actionIndex), event.getY(actionIndex), tapCount,
                        event.getEventTime());
                allEvents.add(ev);
            }
            
            for (int i = 0; i < pointerCount; i++) {
                if ((source & InputDevice.SOURCE_CLASS_JOYSTICK) > 0) {
                    for (int a = 0; a < gamepadAxises.length; ++a) {
                        InputEvent ev = new InputEvent(gamepadAxises[a],
                                event.getAxisValue(gamepadAxises[a], i), 0,
                                tapCount, event.getEventTime());
                        allEvents.add(ev);
                    }
                }
            }
    	}
    	public InputRunnable(final com.bda.controller.MotionEvent event)
    	{
    		action = MotionEvent.ACTION_MOVE;
    		allEvents = new ArrayList<InputEvent>();
    		source = InputDevice.SOURCE_CLASS_JOYSTICK;
        	int pointerCount = event.getPointerCount();
	    	for (int i = 0; i < pointerCount; ++i)
	    	{
	    		//InputEvent::id corresponds to axis id from UIEvent::eJoystickAxisID
	        	allEvents.add(new InputEvent(1, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_X, i), 0, event.getEventTime()));
	        	allEvents.add(new InputEvent(2, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_Y, i), 0, event.getEventTime()));
	        	allEvents.add(new InputEvent(3, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_Z, i), 0, event.getEventTime()));
	        	allEvents.add(new InputEvent(6, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_RZ, i), 0, event.getEventTime()));
	        	allEvents.add(new InputEvent(7, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_LTRIGGER, i), 0, event.getEventTime()));
	        	allEvents.add(new InputEvent(8, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_RTRIGGER, i), 0, event.getEventTime()));
    		}
    		activeEvents = allEvents;
    		groupSize = event.getPointerCount();
    	}

		@Override
		public void run() 
		{
			if ((source & InputDevice.SOURCE_CLASS_JOYSTICK) > 0)
			{
				for(InputEvent event : allEvents)
				{
					if(event.tid == MotionEvent.AXIS_Y || event.tid == MotionEvent.AXIS_RZ || event.tid == MotionEvent.AXIS_RY) 
					{
						nativeOnGamepadElement(event.tid, -event.x, false);
					} 
					else 
					{
						nativeOnGamepadElement(event.tid, event.x, false);
					}
				}
			}
			else if(allEvents.size() != 0) 
			{
				nativeOnInput(action, source, groupSize, allEvents, allEvents);
			}
		}
    }
	
	boolean IsGamepadButton(int keyCode)
	{
		for(Integer o : overridedGamepadKeys) {
			if(o == keyCode) {
				return false;
			}
		}
		
		return KeyEvent.isGamepadButton(keyCode);
	}
	
    class KeyInputRunnable implements Runnable {
    	int keyCode;
    	public KeyInputRunnable(int keyCode) {
    		this.keyCode = keyCode;
    	}
    	
    	@Override
    	public void run() {
    		if(IsGamepadButton(keyCode))
    		{
    			nativeOnGamepadElement(keyCode, 1.f, true);
    		}
    		else
    		{
    			nativeOnKeyDown(keyCode);
    		}
    	}
    }
    
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
    	if(keyCode >= MAX_KEYS) // Ignore too big keycodes
    	{
    		return super.onKeyDown(keyCode, event);
    	}
    	
    	if(pressedKeys[keyCode] == false)
    		queueEvent(new KeyInputRunnable(keyCode));
    	pressedKeys[keyCode] = true;
    	
    	if (event.isSystem())
    		return super.onKeyDown(keyCode, event);
    	else
    		return true;
    }
    
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
    	if(keyCode >= MAX_KEYS) // Ignore too big keycodes
    	{
    		return super.onKeyUp(keyCode, event);
    	}
    	
    	pressedKeys[keyCode] = false;
    	
    	if(IsGamepadButton(keyCode))
    	{
    		nativeOnGamepadElement(keyCode, 0.f, true);
    	}
    	else
    	{
    		nativeOnKeyUp(keyCode);
    	}
    	
    	return super.onKeyUp(keyCode, event);
    }
    
    @Override
    public boolean onTouchEvent(MotionEvent event) 
    {
        boolean isDoubleTap = doubleTapDetector.onTouchEvent(event);
        if (lastDoubleActionIdx >= 0 &&
        	lastDoubleActionIdx == event.getActionIndex() &&
        	event.getAction() == MotionEvent.ACTION_UP) {
        	lastDoubleActionIdx = -1;
        	queueEvent(new InputRunnable(event, 2));
        	isDoubleTap = true;
        }
        if (!isDoubleTap)
            queueEvent(new InputRunnable(event, 1));
        return true;
    }
    
    @Override
    public boolean onGenericMotionEvent(MotionEvent event)
    {
    	queueEvent(new InputRunnable(event, 1));
    	return true;
    }
    
    class MOGAListener implements ControllerListener
    {
    	GLSurfaceView parent = null;
    	
    	MOGAListener(GLSurfaceView parent)
    	{
    		this.parent = parent;
    	}
    	
		@Override
		public void onKeyEvent(com.bda.controller.KeyEvent event)
		{
			int keyCode = event.getKeyCode();
            if(keyCode >= MAX_KEYS) // Ignore too big keycodes
            {
                return;
            }
			if(event.getAction() == com.bda.controller.KeyEvent.ACTION_DOWN)
			{
		    	if(pressedKeys[keyCode] == false)
		    		parent.queueEvent(new KeyInputRunnable(keyCode));
		    	pressedKeys[keyCode] = true;
			}
			else if(event.getAction() == com.bda.controller.KeyEvent.ACTION_UP)
			{
		    	pressedKeys[keyCode] = false;
		        nativeOnGamepadElement(keyCode, 0.f, true);
			}
		}
		@Override
		public void onMotionEvent(com.bda.controller.MotionEvent event)
		{
			parent.queueEvent(new InputRunnable(event));
		}
		@Override
		public void onStateEvent(StateEvent event)
		{
			
		}
    }
}
