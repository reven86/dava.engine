package com.dava.framework;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;


public class JNIGLSurfaceView extends GLSurfaceView 
{
	private JNIRenderer mRenderer = null;

    private static final int TOUCH_SIZE_DELTA = 10;
    
    private Touch []touches = null;
    private int touchesCount = 0;
    

    private native void nativeOnTouch(int action, int id, float x, float y, long time);
    private native void nativeOnKeyUp(int keyCode);
    private native void nativeOnResumeView();
    private native void nativeOnPauseView();

    public JNIGLSurfaceView(Context context) 
    {
        super(context);
        
        touchesCount = 0;
        AllocateTouches(TOUCH_SIZE_DELTA);
     
        this.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        
//        setPreserveEGLContextOnPause(true);
        setEGLContextFactory(new JNIContextFactory());
        setEGLConfigChooser(new JNIConfigChooser(8, 8, 8, 8, 16, 8));
        
        mRenderer = new JNIRenderer();
        setRenderer(mRenderer);
    }
    
    @Override
	public void onPause()
	{
    	queueEvent(new Runnable() 
    	{
    		public void run() 
    		{
    	    	nativeOnPauseView();
    		}
    	});
		super.onPause();
	}

    @Override
	public void onResume()
	{
		super.onResume();
		nativeOnResumeView();
	}
    
    @Override
    public boolean onKeyDown(final int keyCode, final KeyEvent event)
    {
//    	queueEvent(new Runnable() 
//    	{
//    		public void run() 
//    		{
//    		}
//    	});	
    	
    	return super.onKeyDown(keyCode, event);
    }
    
    @Override
    public boolean onKeyUp(final int keyCode, final KeyEvent event)
    {
    	queueEvent(new Runnable() 
    	{
    		public void run() 
    		{
    			int metaState = event.getMetaState(); 
    			int keyChar = event.getUnicodeChar(metaState);
    			if(0 != keyChar)
    			{
//    				Log.d(JNIConst.LOG_TAG, "Char = " + keyChar);
    				nativeOnKeyUp(keyChar);
    			}
    		}
    	});	

    	return super.onKeyUp(keyCode, event);
    }
    

    public boolean onTouchEvent(final MotionEvent event) 
    {
    	queueEvent(new Runnable()
    	{
    		public void run() 
    		{
    			int action = event.getActionMasked();
    			int actionIndex = event.getActionIndex();
    			int pointerCount = event.getPointerCount();
				long time = event.getEventTime();

    			ReallocateTouches(pointerCount);
    			if(MotionEvent.ACTION_CANCEL == action)
    			{
					Log.i(JNIConst.LOG_TAG, "MotionEvent.ACTION_CANCEL: pointerCount = " + pointerCount);
    			}
    			
    			for(int iPointer = 0; iPointer < pointerCount; ++iPointer)
    			{
    				int id = event.getPointerId(iPointer);
    				Touch touch = FindTouchById(id);
    				if(null == touch)
    				{
    					touch = AddTouch(id);
    				}
    				
    				if(		(MotionEvent.ACTION_POINTER_DOWN == action) 
    					|| 	(MotionEvent.ACTION_POINTER_UP == action))
    				{
        				if(actionIndex == iPointer)
        				{
            				touch.action = action;
        				}
    				}
    				else
    				{
        				touch.action = action;
    				}
    				touch.x = event.getX(iPointer);
    				touch.y = event.getY(iPointer);
    			}
    			
				for(int iTouch = 0; iTouch < touchesCount; ++iTouch)
    			{
    				nativeOnTouch(	touches[iTouch].action, touches[iTouch].id, 
									touches[iTouch].x, touches[iTouch].y, time);
    			}
    			
    			RemoveFinishedTouches();
    		}
    	});
    	
        return true;
    }
    
    protected void RemoveFinishedTouches()
    {
    	int newTouchCount = 0;

    	//reassing order
		for(int iTouch = 0; iTouch < touchesCount; ++iTouch)
		{
			if(		(MotionEvent.ACTION_UP == touches[iTouch].action)
				|| 	(MotionEvent.ACTION_POINTER_UP == touches[iTouch].action)
				|| 	(MotionEvent.ACTION_CANCEL == touches[iTouch].action))
			{
				touches[iTouch].Clear();
			}
			else
			{
				if(newTouchCount != iTouch)
				{
					touches[newTouchCount].Copy(touches[iTouch]);
				}
				++newTouchCount;
			}
		}

		touchesCount = newTouchCount;
		
		//clear tail
		for(int iTouch = touchesCount; iTouch < touches.length; ++iTouch)
		{
			touches[iTouch].Clear();
		}
    }
    
    protected Touch AddTouch(int id)
    {
    	if(touches.length < touchesCount)
    	{
    		ReallocateTouches(touches.length + TOUCH_SIZE_DELTA);
    	}

    	Touch newTouch = touches[touchesCount];
    	newTouch.id = id; 
    	++touchesCount;
    	
    	return newTouch;
    }
    
    protected Touch FindTouchById(int id)
    {
    	Touch foundTouch = null;
		for(int iTouch = 0; iTouch < touchesCount; ++iTouch)
		{
			if(touches[iTouch].id == id)
			{
				foundTouch = touches[iTouch];
				break;
			}
		}
		
		return foundTouch;
    }
    
    protected void ReallocateTouches(int requestedCount)
    {
    	if(touches.length < requestedCount)
    	{
    		Touch []newTouches = new Touch[requestedCount];
        	if(null != newTouches)
        	{
        		int iTouch = 0;
        		//allocate & copy old touches
            	for(; iTouch < touches.length; ++iTouch)
            	{
            		newTouches[iTouch] = new Touch();
        			newTouches[iTouch].Copy(touches[iTouch]);
            	}

            	//allocate new touches
            	for(; iTouch < requestedCount; ++iTouch)
            	{
            		newTouches[iTouch] = new Touch();
            	}
            	touches = newTouches;
        	}
    	}
    }

    protected void AllocateTouches(int requestedCount)
    {
    	touches = new Touch[requestedCount];

    	//allocate new touches
    	for(int iTouch = 0; iTouch < requestedCount; ++iTouch)
    	{
    		touches[iTouch] = new Touch();
    	}
    }

    protected class Touch
    {
    	private static final int INVALID_POINTER_ID = -1;
    	public static final int INVALID_ACTION_ID = -1;
    	
    	protected int id = INVALID_POINTER_ID;
    	protected float x = 0.f;
    	protected float y = 0.f;
    	protected int action = INVALID_ACTION_ID;
    	
    	void Copy(Touch inTouch)
    	{
    		if(null != inTouch)
    		{
        		id = inTouch.id;
        		action = inTouch.action;
        		x = inTouch.x;
        		y = inTouch.y;
    		}
    	}
    	
    	void Clear()
    	{
    		id = INVALID_POINTER_ID;
    		x = 0.f;
    		y = 0.f;
    		action = INVALID_ACTION_ID;
    	}
    }
}
