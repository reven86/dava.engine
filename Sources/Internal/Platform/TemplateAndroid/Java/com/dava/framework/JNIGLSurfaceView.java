package com.dava.framework;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.GestureDetector;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.ViewGroup.LayoutParams;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

import android.util.Log;

import com.bda.controller.ControllerListener;
import com.bda.controller.StateEvent;

public class JNIGLSurfaceView extends GLSurfaceView
{
	private JNIRenderer mRenderer = null;

	//private native void nativeOnInput(int action, int id, float x, float y, double time, int source, int tapCount);
    private native void nativeOnInput(int action, ArrayList<InputRunnable.InputEvent> activeInputs, ArrayList<InputRunnable.InputEvent> allInputs, double time);
	private native void nativeOnKeyDown(int keyCode);
	private native void nativeOnKeyUp(int keyCode);
	
	MOGAListener mogaListener = null;

	boolean[] pressedKeys = new boolean[KeyEvent.getMaxKeyCode()];

	public int lastDoubleActionIdx = -1;
	
	class DoubleTapListener extends GestureDetector.SimpleOnGestureListener{
		JNIGLSurfaceView view;
		
		DoubleTapListener(JNIGLSurfaceView view) {
			this.view = view;
		}
		
		@Override
		public boolean onDoubleTap(MotionEvent e) {
			lastDoubleActionIdx = e.getActionIndex();
			
			view.queueEvent(new InputRunnable(e, 2));
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

		//setPreserveEGLContextOnPause(true);
		setEGLContextFactory(new JNIContextFactory());
		setEGLConfigChooser(new JNIConfigChooser());

		mRenderer = new JNIRenderer();
		setRenderer(mRenderer);
		setRenderMode(RENDERMODE_CONTINUOUSLY);
		
		mogaListener = new MOGAListener(this);
		
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB)
		{
			setPreserveEGLContextOnPause(true);
		}
		
		setDebugFlags(0);
		
		doubleTapDetector = new GestureDetector(JNIActivity.GetActivity(), new DoubleTapListener(this));
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
	public void onPause()
	{
		super.onPause();
		setRenderMode(RENDERMODE_WHEN_DIRTY);
		mRenderer.OnPause();
	}
	
	@Override
	public void onResume()
	{
		super.onResume();
		setRenderMode(RENDERMODE_CONTINUOUSLY);
	};

	Map<Integer, Integer> tIdMap = new HashMap<Integer, Integer>();
	int nexttId = 1;
	
	public class InputRunnable implements Runnable
	{
		public class InputEvent
		{
			int id;
            int tid;
			float x;
			float y;
			int source;
			int tapCount;
			
			InputEvent(int id, int tid, float x, float y, int source)
			{
				this.id = id;
                this.tid = tid;
				this.x = x;
				this.y = y;
				this.source = source;
				this.tapCount = 1;
			}
			
			InputEvent(int id, int tid, float x, float y, int source, int tapCount)
			{
				this.id = id;
                this.tid = tid;
				this.x = x;
				this.y = y;
				this.source = source;
				this.tapCount = tapCount;
			}
		}

		ArrayList<InputEvent> activeEvents;
        ArrayList<InputEvent> allEvents;

		double time;
		int action;

		public InputRunnable(final android.view.MotionEvent event, int tapCount)
		{
			activeEvents = new ArrayList<InputEvent>();
            allEvents = new ArrayList<InputEvent>();

			action = event.getActionMasked();
			//if(action == MotionEvent.ACTION_MOVE)
			//{

/*
                if (event.source == InputDevice.SOURCE_CLASS_JOYSTICK) {
                    nativeOnInput(action, event.id + 1, event.x, event.y, time, event.source, event.tapCount);
                } else {
                    if (action != MotionEvent.ACTION_MOVE) {
                        Log.d("DAVA", "InputRunnable.run id: " + event.id + " tid: " + GetTId(event.id) + " action: " + action);
                    }

                    nativeOnInput(action, GetTId(event.id), event.x, event.y, time, event.source, event.tapCount);
                    

                    */
				int pointerCount = event.getPointerCount();
				for (int i = 0; i < pointerCount; ++i)
				{
					if((event.getSource() & InputDevice.SOURCE_CLASS_POINTER) > 0)
					{
                        int pointerIdx = event.getPointerId(i);
                        InputEvent ev = new InputEvent(pointerIdx, GetTId(pointerIdx), event.getX(i), event.getY(i), event.getSource(), tapCount);
						allEvents.add(ev);
                        if(action == MotionEvent.ACTION_MOVE) activeEvents.add(ev);
					}
					if((event.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) > 0)
					{
						//InputEvent::id corresponds to axis id from UIEvent::eJoystickAxisID
                        InputEvent ev = new InputEvent(0, 1, event.getAxisValue(MotionEvent.AXIS_X, i), 0, event.getSource(), tapCount);
						if(action == MotionEvent.ACTION_MOVE) activeEvents.add(ev);
                        allEvents.add(ev);

                        ev = new InputEvent(1, 2, event.getAxisValue(MotionEvent.AXIS_Y, i), 0, event.getSource(), tapCount);
						if(action == MotionEvent.ACTION_MOVE) activeEvents.add(ev);
                        allEvents.add(ev);

                        ev = new InputEvent(2, 3, event.getAxisValue(MotionEvent.AXIS_Z, i), 0, event.getSource(), tapCount);
						if(action == MotionEvent.ACTION_MOVE) activeEvents.add(ev);
                        allEvents.add(ev);

                        ev = new InputEvent(3, 4, event.getAxisValue(MotionEvent.AXIS_RX, i), 0, event.getSource(), tapCount);
						if(action == MotionEvent.ACTION_MOVE) activeEvents.add(ev);
                        allEvents.add(ev);

                        ev = new InputEvent(4, 5, event.getAxisValue(MotionEvent.AXIS_RY, i), 0, event.getSource(), tapCount);
						if(action == MotionEvent.ACTION_MOVE) activeEvents.add(ev);
                        allEvents.add(ev);

                        ev = new InputEvent(5, 6, event.getAxisValue(MotionEvent.AXIS_RZ, i), 0, event.getSource(), tapCount);
						if(action == MotionEvent.ACTION_MOVE) activeEvents.add(ev);
                        allEvents.add(ev);

						ev = new InputEvent(6, 7, event.getAxisValue(MotionEvent.AXIS_LTRIGGER, i), 0, event.getSource(), tapCount);
                        if(action == MotionEvent.ACTION_MOVE) activeEvents.add(ev);
                        allEvents.add(ev);

                        ev = new InputEvent(7, 8, event.getAxisValue(MotionEvent.AXIS_RTRIGGER, i), 0, event.getSource(), tapCount);
						if(action == MotionEvent.ACTION_MOVE) activeEvents.add(ev);
                        allEvents.add(ev);

                        ev = new InputEvent(8, 9, event.getAxisValue(MotionEvent.AXIS_HAT_X, i), 0, event.getSource(), tapCount);
						if(action == MotionEvent.ACTION_MOVE) activeEvents.add(ev);
                        allEvents.add(ev);

                        ev = new InputEvent(9, 10, event.getAxisValue(MotionEvent.AXIS_HAT_Y, i), 0, event.getSource(), tapCount);
						if(action == MotionEvent.ACTION_MOVE) activeEvents.add(ev);
                        allEvents.add(ev);
	    			}
	    		}
    		//}
    		//else
    		//{
            if(action != MotionEvent.ACTION_MOVE) {
    			int actionIdx = event.getActionIndex();
                int pointerIdx = event.getPointerId(actionIdx);
    			assert(actionIdx <= event.getPointerCount());
    			activeEvents.add(new InputEvent(pointerIdx, GetTId(pointerIdx), event.getX(actionIdx), event.getY(actionIdx), event.getSource(), tapCount));
            }

    		//}
    	}
    	public InputRunnable(final com.bda.controller.MotionEvent event)
    	{
            activeEvents = new ArrayList<InputEvent>();
            allEvents = new ArrayList<InputEvent>();
    		action = MotionEvent.ACTION_MOVE;
    		// events = new ArrayList<InputEvent>();
        	int pointerCount = event.getPointerCount();
	    	for (int i = 0; i < pointerCount; ++i)
	    	{
	    		//InputEvent::id corresponds to axis id from UIEvent::eJoystickAxisID
                InputEvent ev = new InputEvent(0, 1, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_X, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK);
	        	allEvents.add(ev);
                activeEvents.add(ev);

                ev = new InputEvent(1, 2, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_Y, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK);
	        	allEvents.add(ev);
                activeEvents.add(ev);

                ev = new InputEvent(2, 3, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_Z, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK);
                allEvents.add(ev);
	        	activeEvents.add(ev);

                ev = new InputEvent(5, 6, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_RZ, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK);
	        	allEvents.add(ev);
                activeEvents.add(ev);

	        	ev = new InputEvent(6, 7, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_LTRIGGER, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK);
                allEvents.add(ev);
                activeEvents.add(ev);

                ev = new InputEvent(7, 8, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_RTRIGGER, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK);
	        	allEvents.add(ev);
                activeEvents.add(ev);
    		}
    	}
    	
    	int GetTId(int id) {
    		if (tIdMap.containsKey(id)) 
    			return tIdMap.get(id);
    		
    		int tId = nexttId++;
    		tIdMap.put(id, tId);

            Log.d("DAVA", "InputRunnable.GetTId tid: " +  tIdMap.get(id) + " id: " + id);

    		return tId;
    	}
    	
    	void RemoveTId(int id) {
            Log.d("DAVA", "InputRunnable.RemoveTId tid: " +  tIdMap.get(id) + " id: " + id);
    		tIdMap.remove(id);
    	}

		@Override
		public void run() {
            nativeOnInput(action, activeEvents, allEvents, time);
			/*for (int i = 0; i < events.size(); ++i) {
				InputEvent event = events.get(i);
				
				if (event.source == InputDevice.SOURCE_CLASS_JOYSTICK) {
					nativeOnInput(action, event.id + 1, event.x, event.y, time, event.source, event.tapCount);
				} else {
                    if (action != MotionEvent.ACTION_MOVE) {
                        Log.d("DAVA", "InputRunnable.run id: " + event.id + " tid: " + GetTId(event.id) + " action: " + action);
                    }

					nativeOnInput(action, GetTId(event.id), event.x, event.y, time, event.source, event.tapCount);
					
					if (action == MotionEvent.ACTION_CANCEL ||
						action == MotionEvent.ACTION_UP ||
						action == MotionEvent.ACTION_POINTER_1_UP ||
						action == MotionEvent.ACTION_POINTER_2_UP ||
						action == MotionEvent.ACTION_POINTER_3_UP) {
						RemoveTId(event.id);
					}
				}
			}*/
            if (action == MotionEvent.ACTION_CANCEL ||
                        action == MotionEvent.ACTION_UP ||
                        action == MotionEvent.ACTION_POINTER_1_UP ||
                        action == MotionEvent.ACTION_POINTER_2_UP ||
                        action == MotionEvent.ACTION_POINTER_3_UP) {

                Log.d("DAVA", "UP --- begin");
                for (int i = 0; i < activeEvents.size(); ++i) {
                    InputEvent event = activeEvents.get(i);
                    Log.d("DAVA", "event id " + event.id);
                    RemoveTId(event.id);
                }

                Log.d("DAVA", "UP --- all");

                for (int i = 0; i < allEvents.size(); ++i) {
                    InputEvent event = allEvents.get(i);
                    Log.d("DAVA", "event id " + event.id);
                }

                Log.d("DAVA", "UP --- end");
            }
		}
    }

    class KeyInputRunnable implements Runnable {
    	int keyCode;
    	public KeyInputRunnable(int keyCode) {
    		this.keyCode = keyCode;
    	}
    	
    	@Override
    	public void run() {
    		nativeOnKeyDown(keyCode);
    	}
    }
    
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
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
    	pressedKeys[keyCode] = false;
        nativeOnKeyUp(keyCode);
    	return super.onKeyUp(keyCode, event);
    }
    
    @Override
    public boolean onTouchEvent(MotionEvent event) 
    {
        int action = event.getActionMasked();
        if (action != MotionEvent.ACTION_MOVE) {
            int pointerCount = event.getPointerCount();
            for (int i = 0; i < pointerCount; ++i) {
                Log.d("DAVA", "JNIGLSurfaceView.onTouchEvent id: " +  event.getPointerId(i) + " action: " + action);
            }
        }

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
			if(event.getAction() == com.bda.controller.KeyEvent.ACTION_DOWN)
			{
		    	if(pressedKeys[keyCode] == false)
		    		parent.queueEvent(new KeyInputRunnable(keyCode));
		    	pressedKeys[keyCode] = true;
			}
			else if(event.getAction() == com.bda.controller.KeyEvent.ACTION_UP)
			{
		    	pressedKeys[keyCode] = false;
		        nativeOnKeyUp(keyCode);
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
