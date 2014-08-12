package com.dava.framework;

import android.app.Activity;
import android.content.Context;
import android.hardware.SensorManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Bundle;
import android.os.Handler;
import android.telephony.TelephonyManager;
import android.util.Log;
import org.fmod.FMODAudioDevice;
import com.bda.controller.Controller;

public abstract class JNIActivity extends Activity implements JNIAccelerometer.JNIAccelerometerListener
{
	private static int errorState = 0;

	private JNIAccelerometer accelerometer = null;
	protected JNIGLSurfaceView glView = null;

	private FMODAudioDevice fmodDevice = new FMODAudioDevice();
	
	private Controller mController;
	
	private native void nativeOnCreate(boolean isFirstRun);
	private native void nativeOnStart();
	private native void nativeOnStop();
	private native void nativeOnDestroy();
	private native void nativeIsFinishing();
	private native void nativeOnAccelerometer(float x, float y, float z);
    
    private boolean isFirstRun = true;
    
    public abstract JNIGLSurfaceView GetSurfaceView();
    
    private static JNIActivity activity = null;
    protected static SingalStrengthListner singalStrengthListner = null;
    
    public static JNIActivity GetActivity()
	{
		return activity;
	}
    
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
    	activity = this;
        super.onCreate(savedInstanceState);
        
        if(null != savedInstanceState)
        {
        	isFirstRun = savedInstanceState.getBoolean("isFirstRun");
        }
        
    	// The activity is being created.
        Log.i(JNIConst.LOG_TAG, "[Activity::onCreate]");

        // initialize accelerometer
        SensorManager sensorManager = (SensorManager)getSystemService(Context.SENSOR_SERVICE);
        accelerometer = new JNIAccelerometer(this, sensorManager);

        // initialize GL VIEW
        glView = GetSurfaceView();
        assert(glView != null);
        glView.setFocusableInTouchMode(true);
        glView.setClickable(true);
        glView.setFocusable(true);
        glView.requestFocus();

        mController = Controller.getInstance(this);
        if(mController != null)
        {
        	mController.init();
        	mController.setListener(glView.mogaListener, new Handler());
        }

        Log.i(JNIConst.LOG_TAG, "[Activity::onCreate] isFirstRun is " + isFirstRun); 
        nativeOnCreate(isFirstRun);
        
        JNITextField.RelinkNativeControls();
        JNIWebView.RelinkNativeControls();
        
        try {
        	ConnectivityManager cm = (ConnectivityManager)getSystemService(CONNECTIVITY_SERVICE);
        	NetworkInfo networkInfo = cm.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
        	if (cm != null && networkInfo != null && networkInfo.isConnectedOrConnecting())
            {
            	TelephonyManager tm = (TelephonyManager)getSystemService(TELEPHONY_SERVICE);
            	singalStrengthListner = new SingalStrengthListner();
            	tm.listen(singalStrengthListner, SingalStrengthListner.LISTEN_SIGNAL_STRENGTHS);
            }
		} catch (Exception e) {
			Log.d("", "no singalStrengthListner");
		}
    }
    
    @Override
    protected void onStart()
    {
    	super.onStart();
    	fmodDevice.start();
    	// The activity is about to become visible.
    	
        Log.i(JNIConst.LOG_TAG, "[Activity::onStart]");
        
        //editText.setVisibility(EditText.INVISIBLE);
        
        //call native method
        nativeOnStart();
    }
    
    @Override
    protected void onRestart()
    {
    	super.onRestart();
    	// The activity is about to become visible.
    	
        Log.i(JNIConst.LOG_TAG, "[Activity::onRestart]");
        
        //TODO: VK: may be usefull
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) 
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onSaveInstanceState]");

        outState.putBoolean("isFirstRun", isFirstRun);
    	
    	super.onSaveInstanceState(outState);
    }
    
    @Override
    protected void onResume() 
    {
        super.onResume();
        // The activity has become visible (it is now "resumed").
		Log.i(JNIConst.LOG_TAG, "[Activity::onResume] start");

		if(mController != null)
		{
			mController.onResume();
		}
		
        // activate accelerometer
        if(null != accelerometer)
        {
        	if(accelerometer.IsSupported())
        	{
        		accelerometer.Start();
        	}
        }
        
        // activate GLView 
        if(null != glView)
        {
        	glView.onResume();
        }
        
        Log.i(JNIConst.LOG_TAG, "[Activity::onResume] finish");
    }

    
    @Override
    protected void onPause() 
    {
        super.onPause();
        // Another activity is taking focus (this activity is about to be "paused").

        Log.i(JNIConst.LOG_TAG, "[Activity::onPause] start");

		if(mController != null)
		{
			mController.onPause();
		}
		
        // deactivate accelerometer
        if(null != accelerometer)
        {
        	if(accelerometer.IsActive())
        	{
        		accelerometer.Stop();
        	}
        }

        // deactivate GLView 
        if(null != glView)
        {
        	glView.onPause();
        }
        
        boolean isActivityFinishing = isFinishing();
        Log.i(JNIConst.LOG_TAG, "[Activity::onPause] isActivityFinishing is " + isActivityFinishing);
        if(isActivityFinishing)
        {
        	nativeIsFinishing();
        }

        Log.i(JNIConst.LOG_TAG, "[Activity::onPause] finish");
    }

    @Override
    protected void onStop()
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onStop] start");
        
        fmodDevice.stop();
        
        //call native method
        nativeOnStop();

        Log.i(JNIConst.LOG_TAG, "[Activity::onStop] finish");
        
        super.onStop();
    	// The activity is no longer visible (it is now "stopped")
    }
    
    
    @Override
    protected void onDestroy()
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onDestroy] start");

        if(mController != null)
        	mController.exit();
        
        //call native method
        nativeOnDestroy();

        Log.i(JNIConst.LOG_TAG, "[Activity::onDestroy] finish");

        super.onDestroy();
    	// The activity is about to be destroyed.
    }

    @Override
    protected void onPostResume() 
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onPostResume] ****");
        
    	super.onPostResume();
    }
    
    @Override
    public void onBackPressed() {
    	
    }
    
    public void onAccelerationChanged(float x, float y, float z)
	{
		nativeOnAccelerometer(x, y, z);
	}
	
	public void PostEventToGL(Runnable event) {
		glView.queueEvent(event);
	}
}
