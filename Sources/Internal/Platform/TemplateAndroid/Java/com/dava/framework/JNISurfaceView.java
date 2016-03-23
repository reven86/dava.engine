package com.dava.framework;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.util.Log;
import android.util.Pair;
import android.view.GestureDetector;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

public class JNISurfaceView extends SurfaceView implements SurfaceHolder.Callback
{
	private static final int MAX_KEYS = 256; // Maximum number of keycodes which used in native code

	private native void nativeOnInput(int action, int source, int groupSize, ArrayList< InputRunnable.InputEvent > activeInputs, ArrayList< InputRunnable.InputEvent > allInputs);
	private native void nativeOnKeyDown(int keyCode);
	private native void nativeOnKeyUp(int keyCode);
	private native void nativeOnGamepadElement(int elementKey, float value, boolean isKeycode);

	private native void nativeSurfaceCreated(Surface surface);
	private native void nativeSurfaceChanged(Surface surface, int width, int height);
	private native void nativeSurfaceDestroyed();

    private native void nativeProcessFrame();

    // Make surface member as static due to JNISurfaceView's lifecycle
    // System can create new JNISurfaceView instance before deleting previous instance
    // So use surface as current surface
    // TODO: work with surface in SDL way 
	static private Surface surface = null;
	private int surfaceWidth = 0, surfaceHeight = 0;

	private boolean isMultitouchEnabled = true;
	
	private Integer[] gamepadAxises = null;
	private Integer[] overridedGamepadKeys = null;
	private ArrayList< Pair<Integer, Integer> > gamepadButtonsAxisMap = new ArrayList< Pair<Integer, Integer> >();
	
	private ArrayList<Runnable> mEventQueue = new ArrayList<Runnable>();
	private volatile boolean mEventQueueReady = true;

	public int lastDoubleActionIdx = -1;
	
	class DoubleTapListener extends GestureDetector.SimpleOnGestureListener{
		JNISurfaceView surfaceView;
		
		DoubleTapListener(JNISurfaceView view) {
			this.surfaceView = view;
		}
		
		@Override
		public boolean onDoubleTap(MotionEvent e) {
			lastDoubleActionIdx = e.getActionIndex();
			
			surfaceView.queueEvent(new InputRunnable(e, 2));
			return true;
		}
	}
	
	GestureDetector doubleTapDetector = null;

	public JNISurfaceView(Context context) 
	{
		super(context);
		Init();
	}

	public JNISurfaceView(Context context, AttributeSet attrs)
	{
		super(context, attrs);
		Init();
	}

	private void Init()
	{
		getHolder().setFormat(PixelFormat.TRANSLUCENT);
		
		doubleTapDetector = new GestureDetector(JNIActivity.GetActivity(), new DoubleTapListener(this));

		gamepadButtonsAxisMap.add(new Pair<Integer, Integer>(KeyEvent.KEYCODE_BUTTON_L2, MotionEvent.AXIS_LTRIGGER));
		gamepadButtonsAxisMap.add(new Pair<Integer, Integer>(KeyEvent.KEYCODE_BUTTON_L2, MotionEvent.AXIS_BRAKE));
		gamepadButtonsAxisMap.add(new Pair<Integer, Integer>(KeyEvent.KEYCODE_BUTTON_R2, MotionEvent.AXIS_RTRIGGER));
		gamepadButtonsAxisMap.add(new Pair<Integer, Integer>(KeyEvent.KEYCODE_BUTTON_R2, MotionEvent.AXIS_GAS));
		
        getHolder().addCallback(this);
	}
	
	public void ProcessQueuedEvents()
	{
		ArrayList<Runnable> queueCopy = null;

		synchronized (mEventQueue) {
			queueCopy = new ArrayList<Runnable>(mEventQueue);
			mEventQueue.clear();
		}
		
		for(Runnable r : queueCopy) {
    		r.run();
    	}

    	synchronized(mEventQueue) {
    		mEventQueueReady = true;
    		mEventQueue.notify();
    	}
	}

	public void WaitQueuedEvents()
	{
    	synchronized(mEventQueue)
    	{
            if(!mEventQueue.isEmpty()) {
    		    mEventQueueReady = false;
    		    while(!mEventQueueReady) {
                    try {
	    			    mEventQueue.wait();
                    } catch(InterruptedException e) {
                        e.printStackTrace();
                    }
    		    }
            }
		}		
	}

	public void ProcessFrame()
	{
        if (!JNIAssert.waitUserInputOnAssertDialog)
        {
            nativeProcessFrame();
        }
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
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) 
    {
        // Fix lag when text field lost focus, but keyboard not closed yet. 
        outAttrs.imeOptions = JNITextField.GetLastKeyboardIMEOptions();
        outAttrs.inputType = JNITextField.GetLastKeyboardInputType();
        return super.onCreateInputConnection(outAttrs);
    }
    
	@Override
	protected void onSizeChanged(int w, int h, int oldw, int oldh) 
	{
        // On some devices (e.g. Samsung SM-G900F with Android 5) when
        // starting app from notification label on lock screen
        // method onSizeChanged is called with dimension like
        // in portrait mode despite of landscape orientation in AndroidManifest.xml.
		// So tell superclass of our expected and desired width and height, hehe
        // See also method surfaceChanged
		if (w < h)
		{
			int temp = w;
			w = h;
			h = temp;
		}
		super.onSizeChanged(w, h, oldw, oldh);
	}
	
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

		public InputRunnable(final android.view.MotionEvent event, final int tapCount) {
			allEvents = new ArrayList<InputEvent>();

			action = event.getActionMasked();
			source = event.getSource();
			groupSize = 1;

			final int pointerCount = event.getPointerCount();

			if ((source & InputDevice.SOURCE_CLASS_JOYSTICK) > 0) {

				final int historySize = event.getHistorySize();

				for (int historyStep = 0; historyStep < historySize; historyStep++) {
					for (int i = 0; i < pointerCount; i++) {
						for (int a = 0; a < gamepadAxises.length; ++a) {
							InputEvent ev = new InputEvent(gamepadAxises[a],
									event.getHistoricalAxisValue(gamepadAxises[a], i, historyStep), 0, tapCount,
									event.getHistoricalEventTime(historyStep));

							allEvents.add(ev);
						}
					}
				}

				for (int i = 0; i < pointerCount; i++) {
					for (int a = 0; a < gamepadAxises.length; ++a) {
						InputEvent ev = new InputEvent(gamepadAxises[a], event.getAxisValue(gamepadAxises[a], i), 0,
								tapCount, event.getEventTime());
						allEvents.add(ev);
					}
				}
			} else if ((source & InputDevice.SOURCE_CLASS_POINTER) > 0) {
				int actionIndex = event.getActionIndex();
				int pointerId = event.getPointerId(actionIndex);
				if (android.view.MotionEvent.ACTION_MOVE == action
						|| android.view.MotionEvent.ACTION_CANCEL == action) {

					groupSize = pointerCount;

					for (int i = 0; i < pointerCount; i++) {
						pointerId = event.getPointerId(i);
						if (isMultitouchEnabled) {
							InputEvent ev = new InputEvent(touchIdForPointerId(pointerId), event.getX(i), event.getY(i),
									tapCount, event.getEventTime());
							allEvents.add(ev);
						}
					}
				} else {
					InputEvent ev = new InputEvent(touchIdForPointerId(pointerId), event.getX(actionIndex),
							event.getY(actionIndex), tapCount, event.getEventTime());
					allEvents.add(ev);
				}
			} else
			{
				Log.d(JNIConst.LOG_TAG, "unsupported moution input source: " + source);
			}
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
    	boolean isDown;
    	
    	public KeyInputRunnable(int keyCode, boolean isDown) {
    		this.keyCode = keyCode;
    		this.isDown = isDown;
    	}
    	
    	@Override
    	public void run() {
    		if(IsGamepadButton(keyCode))
    		{
    			nativeOnGamepadElement(keyCode, isDown ? 1 : 0, true);
    		}
    		else
    		{
    			if (isDown)
    			{
    				nativeOnKeyDown(keyCode);
    			} else
    			{
    				nativeOnKeyUp(keyCode);
    			}
    		}
    	}
    }
    
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
    	if(keyCode >= MAX_KEYS) // Ignore too big Android keycodes
    	{
    		return super.onKeyDown(keyCode, event);
    	}
    	
    	queueEvent(new KeyInputRunnable(keyCode, true));
    	
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
    	
    	queueEvent(new KeyInputRunnable(keyCode, false));
    	
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
    
    public void queueEvent(Runnable r)
    {
        synchronized(mEventQueue)
        {
            mEventQueue.add(r);
        }
    }

    public void surfaceCreated(SurfaceHolder holder)
    {
    	if (surface != null)
    	{
    		Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceCreated: previous surface is alive, call nativeSurfaceDestroyed");
    		queueEvent(new Runnable() {
    			public void run() {
                    Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceCreated runnable in: call nativeSurfaceDestroyed");
    		    	nativeSurfaceDestroyed();
                    Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceCreated runnable out: call nativeSurfaceDestroyed");
    			}
    		});
    	}
    	
        Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceCreated in");
    	surface = holder.getSurface();
    	surfaceWidth = surfaceHeight = 0;
    	
    	queueEvent(new Runnable() {
			public void run() {
                Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceCreated runnable in");
		    	nativeSurfaceCreated(surface);
                Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceCreated runnable out");
			}
		});

        Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceCreated out");
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
    {
    	if (surface != holder.getSurface())
    	{
    		Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceChanged for previous object! Do nothing");
    		return;
    	}
    	
        Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceChanged in");

        // while we always in landscape mode, but some devices
        // call this method on lock screen with portrait w and h
        // then call this method second time with correct portrait w and h
        // I assume width always more then height
        // we need skip portrait w and h because text in engine can be
        // pre render into texture with incorrect aspect and on second
        // call with correct w and h such text textures stays incorrect
        // http://stackoverflow.com/questions/8556332/is-it-safe-to-assume-that-in-landscape-mode-height-is-always-less-than-width
        // DF-5068
        // strange but after add to manifest.xml screenSize to config
        // on nexus 5 w == h == 1080
        // if you have any trouble here you should first check
        // res/layout/activity_main.xml and root layout is FrameLayout!
        
        // On some devices (e.g. Samsung SM-G900F with Android 5) when
        // starting app from notification label on lock screen
        // method surfaceChanged is called only once with dimension like
        // in portrait mode despite of landscape orientation in AndroidManifest.xml.
        // See also method onSizeChanged
        if (width < height)
        {
            int temp = width;
            width = height;
            height = temp;
        }
        
        {
            if (width != surfaceWidth || height != surfaceHeight)
            {
            	surfaceWidth = width;
            	surfaceHeight = height;

            	queueEvent(new Runnable() {
        			public void run() {
                        Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceChanged runnable in");
        		    	nativeSurfaceChanged(surface, surfaceWidth, surfaceHeight);
                        Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceChanged runnable out");
        			}
        		});

                // Workaround! we have to initialize keyboard after glView(OpenGL)
                // initialization for some devices like
                // HTC One (adreno 320, os 4.3)
                final JNIActivity activity = JNIActivity.GetActivity();
                activity.runOnUiThread(new Runnable(){
                    @Override
                    public void run() {
                        activity.InitKeyboardLayout();
                    }
                });

        		WaitQueuedEvents();
            }
        }

		JNIActivity.isSurfaceReady = true;

        Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceChanged out");
    }
    
    public void surfaceDestroyed(SurfaceHolder holder)
    {
    	if (surface != holder.getSurface())
    	{
    		Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceDestroyed for previous object! Do nothing");
    		return;
    	}
    	
        Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceDestroyed in");
    	queueEvent(new Runnable() {
			public void run() {
                Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceDestroyed runnable in");
		    	nativeSurfaceDestroyed();
                Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceDestroyed runnable out");
			}
		});

		WaitQueuedEvents();

		JNIActivity.isSurfaceReady = false;

        surface = null;
        Log.d(JNIConst.LOG_TAG, "JNISurfaceView surfaceDestroyed out");
    }
}
