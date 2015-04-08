package com.dava.framework;

import java.util.Arrays;
import java.util.List;
import java.util.HashSet;
import java.util.Set;

import org.fmod.FMODAudioDevice;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.hardware.SensorManager;
import android.hardware.input.InputManager;
import android.hardware.input.InputManager.InputDeviceListener;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnAttachStateChangeListener;
import android.view.Window;
import android.view.WindowManager;
import android.view.InputDevice.MotionRange;

import com.bda.controller.Controller;

public abstract class JNIActivity extends Activity implements JNIAccelerometer.JNIAccelerometerListener, InputDeviceListener
{
	private static int errorState = 0;

	private JNIAccelerometer accelerometer = null;
	protected JNIGLSurfaceView glView = null;
	private View splashView = null;
	
	private FMODAudioDevice fmodDevice = new FMODAudioDevice();
	
	private Controller mController = null;
	
	private InputManager inputManager = null;
	
	private native void nativeOnCreate(boolean isFirstRun);
	private native void nativeOnStart();
	private native void nativeOnStop();
	private native void nativeOnDestroy();
	private native void nativeFinishing();
	private native void nativeOnAccelerometer(float x, float y, float z);
	private native void nativeOnGamepadAvailable(boolean isAvailable);
	private native void nativeOnGamepadTriggersAvailable(boolean isAvailable);
	private native boolean nativeIsMultitouchEnabled();
    
    private boolean isFirstRun = true;
    private static String commandLineParams = null;
    
	public abstract JNIGLSurfaceView GetSurfaceView();
    
    private static JNIActivity activity = null;
    protected static SingalStrengthListner singalStrengthListner = null;
    private boolean isPausing = false;
    
    public boolean GetIsPausing()
    {
        return isPausing;
    }

    public static JNIActivity GetActivity()
	{
		return activity;
	}
    
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
        // The activity is being created.
        Log.i(JNIConst.LOG_TAG, "[Activity::onCreate] start");
        
    	activity = this;
        super.onCreate(savedInstanceState);
        
        commandLineParams = initCommandLineParams();

        // Initialize native framework core         
        JNIApplication.GetApplication().InitFramework(commandLineParams);
        
        //JNINotificationProvider.AttachToActivity();
        
        if(null != savedInstanceState)
        {
        	isFirstRun = savedInstanceState.getBoolean("isFirstRun");
        }
        
        // initialize accelerometer
        SensorManager sensorManager = (SensorManager)getSystemService(Context.SENSOR_SERVICE);
        accelerometer = new JNIAccelerometer(this, sensorManager);

        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_NOTHING | WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN | WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().requestFeature(Window.FEATURE_ACTION_MODE_OVERLAY);
        getWindow().requestFeature(Window.FEATURE_ACTION_BAR_OVERLAY);
        getWindow().requestFeature(Window.FEATURE_NO_TITLE);
        
        final View decorView = getWindow().getDecorView();
		// Try hide navigation bar for detect correct GL view size
        HideNavigationBar(decorView);
        // Subscribe listener on UI changing for hiding navigation bar after keyboard hiding
        decorView.setOnSystemUiVisibilityChangeListener(new View.OnSystemUiVisibilityChangeListener() {
        	@Override
			public void onSystemUiVisibilityChange(int visibility) {
				if((visibility & View.SYSTEM_UI_FLAG_HIDE_NAVIGATION) == 0) {
					HideNavigationBar(decorView);
				}
			}
		});
        
        // initialize GL VIEW
        glView = GetSurfaceView();
        assert(glView != null);
        glView.setFocusableInTouchMode(true);
        glView.setClickable(true);
        glView.setFocusable(true);
        glView.requestFocus();
        
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN)
        {
            inputManager = (InputManager)getSystemService(Context.INPUT_SERVICE);
        }
        UpdateGamepadAxises();
        
        splashView = GetSplashView();
        
        if(mController != null)
        {
            if( Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP )
            {		
        	    MogaFixForLollipop.init(mController, this);
			}
            else
            {
                mController.init();
            }
        	mController.setListener(glView.mogaListener, new Handler());
        }
        
        Log.i(JNIConst.LOG_TAG, "[Activity::onCreate] isFirstRun is " + isFirstRun); 
        nativeOnCreate(isFirstRun);

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
        
        JNINotificationProvider.AttachToActivity(this);
        
		Intent intent = getIntent();
		if (null != intent) {
			String uid = intent.getStringExtra("uid");
			if (uid != null) {
				JNINotificationProvider.NotificationPressed(uid);
			}
		}
		
		if (splashView != null)
		{
		    splashView.setVisibility(View.GONE);
		}
        // The activity is being created.
        Log.i(JNIConst.LOG_TAG, "[Activity::onCreate] finish");
    }
    
	private String initCommandLineParams() {
		String commandLine = "";
		Bundle extras = this.getIntent().getExtras();
		if (extras != null) {
			commandLine = "";
			for (String key : extras.keySet()) {
				String value = extras.getString(key);
				commandLine += key + " " + value + " ";
			}
			commandLine = commandLine.trim();
		}
		Log.i(JNIConst.LOG_TAG, "command line params: " + commandLine);
		return commandLine;
	}
    
    @Override
    protected void onStart()
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onStart] start");
    	super.onStart();
    	fmodDevice.start();
        nativeOnStart();
        Log.i(JNIConst.LOG_TAG, "[Activity::onStart] finish");
    }
    
    @Override
    protected void onRestart()
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onRestart] start");
        super.onRestart();
        Log.i(JNIConst.LOG_TAG, "[Activity::onRestart] start");
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) 
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onSaveInstanceState] start");

        outState.putBoolean("isFirstRun", isFirstRun);
    	
    	super.onSaveInstanceState(outState);
    	Log.i(JNIConst.LOG_TAG, "[Activity::onSaveInstanceState] finish");
    }
    
    @Override
    protected void onPause()
    {
        // reverse order of onResume
        // Another activity is taking focus (this activity is about to be "paused").
        Log.i(JNIConst.LOG_TAG, "[Activity::onPause] start");
        isPausing = true;

        if(mController != null)
        {
            mController.onPause();
        }
        
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN)
        {
        	inputManager.unregisterInputDeviceListener(this);
        }
        
        // deactivate accelerometer
        if(accelerometer != null)
        {
            if(accelerometer.IsActive())
            {
                accelerometer.Stop();
            }
        }
        
        boolean isActivityFinishing = isFinishing();
        Log.i(JNIConst.LOG_TAG, "[Activity::onPause] isActivityFinishing is " + isActivityFinishing);
        
        // can destroy eglContext
        // we need to stop rendering before quit application because some objects could became invalid after
        // "nativeFinishing" call.
        glView.onPause();
        
        if(isActivityFinishing)
        {
        	nativeFinishing();
        }
        
        // Destroy keyboard helper window
        JNITextField.DestroyKeyboardLayout(getWindowManager());
        
        super.onPause();

        Log.i(JNIConst.LOG_TAG, "[Activity::onPause] finish");
    }
    
    @Override
    protected void onResume() 
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onResume] start");
        // recreate eglContext (also eglSurface, eglScreen) should be first
        super.onResume();

        // activate accelerometer
        if(accelerometer != null)
        {
            if(accelerometer.IsSupported())
            {
                accelerometer.Start();
            }
        }
        
        if(mController != null)
        {
            mController.onResume();
        }

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN)
        {
        	inputManager.registerInputDeviceListener(this, null);
        }
        UpdateGamepadAxises();
        
        JNIUtils.keepScreenOnOnResume();
        
        {
            glView.onResume();
            
            // Create keyboard layout over glView window.
            if(glView.getWindowToken() != null)
            {
                JNITextField.InitializeKeyboardLayout(getWindowManager(), glView.getWindowToken());
            }
            else
            {
                glView.addOnAttachStateChangeListener(new OnAttachStateChangeListener() {
                    @Override
                    public void onViewDetachedFromWindow(View v) {}
                    
                    @Override
                    public void onViewAttachedToWindow(View v) {
                        JNITextField.InitializeKeyboardLayout(getWindowManager(), glView.getWindowToken());
                        glView.removeOnAttachStateChangeListener(this);
                    }
                });
                
            }
        }
        
        isPausing = false;
        Log.i(JNIConst.LOG_TAG, "[Activity::onResume] finish");
    }

    @Override
    protected void onStop()
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onStop] start");
        
        //call native method
        nativeOnStop();
        
        fmodDevice.stop();
        
        super.onStop();
        
        ShowSplashScreenView();
    	// The activity is no longer visible (it is now "stopped")
        Log.i(JNIConst.LOG_TAG, "[Activity::onStop] finish");
    }
    
    
    @Override
    protected void onDestroy()
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onDestroy] start");

        if(mController != null)
        {
            mController.exit();
        }
        //call native method
        nativeOnDestroy();

        super.onDestroy();
        Log.i(JNIConst.LOG_TAG, "[Activity::onDestroy] finish");
    	// The activity is about to be destroyed.
    }
    
    @Override
    public void onBackPressed() {
    }
    
    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        Log.i(JNIConst.LOG_TAG, "[Activity::onWindowFocusChanged] start");
        // clear key tracking state, so should always be called
        // now we definitely shown on screen
        // http://developer.android.com/reference/android/app/Activity.html#onWindowFocusChanged(boolean)
        super.onWindowFocusChanged(hasFocus);
        
    	if(hasFocus) {
    		HideNavigationBar(getWindow().getDecorView());
    	}
    	Log.i(JNIConst.LOG_TAG, "[Activity::onWindowFocusChanged] finish");
    }
    
    protected final List<Integer> supportedAxises = Arrays.asList(
			MotionEvent.AXIS_X,
			MotionEvent.AXIS_Y,
			MotionEvent.AXIS_Z,
			MotionEvent.AXIS_RX,
			MotionEvent.AXIS_RY,
			MotionEvent.AXIS_RZ,
			MotionEvent.AXIS_LTRIGGER,
			MotionEvent.AXIS_RTRIGGER,
			MotionEvent.AXIS_BRAKE,
			MotionEvent.AXIS_GAS
	);
    
    protected void UpdateGamepadAxises()
    {
    	boolean isGamepadAvailable = false;
		int[] inputDevices = InputDevice.getDeviceIds();
		Set<Integer> avalibleAxises = new HashSet<Integer>(); 
		for(int id : inputDevices)
		{
			if((InputDevice.getDevice(id).getSources() & InputDevice.SOURCE_CLASS_JOYSTICK) > 0)
			{
				isGamepadAvailable = true;
				
				List<MotionRange> ranges = InputDevice.getDevice(id).getMotionRanges();
				for(MotionRange r : ranges)
				{
					int axisId = r.getAxis();
					if(supportedAxises.contains(axisId))
						avalibleAxises.add(axisId);
				}
			}
		}
		
		glView.SetAvailableGamepadAxises(avalibleAxises.toArray(new Integer[0]));
		
		nativeOnGamepadAvailable(isGamepadAvailable);
		nativeOnGamepadTriggersAvailable(avalibleAxises.contains(MotionEvent.AXIS_LTRIGGER) || avalibleAxises.contains(MotionEvent.AXIS_BRAKE));
    }
    
	@Override
	public void onInputDeviceAdded(int deviceId) 
	{
		UpdateGamepadAxises();
	}

	@Override
	public void onInputDeviceChanged(int deviceId) 
	{
		UpdateGamepadAxises();
	}

	@Override
	public void onInputDeviceRemoved(int deviceId) 
	{
		UpdateGamepadAxises();
	}
    
    public void onAccelerationChanged(float x, float y, float z)
	{
		nativeOnAccelerometer(x, y, z);
	}
	
	public void PostEventToGL(Runnable event) {
		glView.queueEvent(event);
	}

	public int GetNotificationIcon() {
        return android.R.drawable.sym_def_app_icon;
    }
	
	/**
	 * Since API 19 we can hide Navigation bar (Immersive Full-Screen Mode)
	 */
    public static void HideNavigationBar(View view) {
    	// The UI options currently enabled are represented by a bitfield.
        // getSystemUiVisibility() gives us that bitfield.
        int uiOptions = view.getSystemUiVisibility();
        
        // Navigation bar hiding:  Backwards compatible to ICS.
        // Don't use View.SYSTEM_UI_FLAG_HIDE_NAVIGATION on API less that 19 because any
        // click on view shows navigation bar, and we must hide it manually only. It is
        // bad workflow.

        // Status bar hiding: Backwards compatible to Jellybean
        if (Build.VERSION.SDK_INT >= 16) {
            uiOptions |= 0x00000004; //View.SYSTEM_UI_FLAG_FULLSCREEN;
        }

        // Immersive mode: Backward compatible to KitKat.
        // Note that this flag doesn't do anything by itself, it only augments the behavior
        // of HIDE_NAVIGATION and FLAG_FULLSCREEN.  For the purposes of this sample
        // all three flags are being toggled together.
        // Note that there are two immersive mode UI flags, one of which is referred to as "sticky".
        // Sticky immersive mode differs in that it makes the navigation and status bars
        // semi-transparent, and the UI flag does not get cleared when the user interacts with
        // the screen.
        if (Build.VERSION.SDK_INT >= 19) {
        	uiOptions |= View.SYSTEM_UI_FLAG_HIDE_NAVIGATION 
        			| 0x00000200 //View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
        			| 0x00000100 //View.SYSTEM_UI_FLAG_LAYOUT_STABLE
			        | 0x00000400 //View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
			        | 0x00001000; //View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
        }
    	
		view.setSystemUiVisibility(uiOptions);
	}
	
	public View GetSplashView() {
		return null;
	}
	
	protected void ShowSplashScreenView() {
    	runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (splashView != null) {
				    Log.i(JNIConst.LOG_TAG, "splashView set visible");
				    splashView.setVisibility(View.VISIBLE);
				    //splashView.bringToFront();
				    JNITextField.HideAllTextFields();
				    JNIWebView.HideAllWebViews();
				}
			}
		});
	}
	
	protected void HideSplashScreenView() {
		runOnUiThread(new Runnable() {
			
			@Override
			public void run() {
				if (splashView != null) {
				    Log.i(JNIConst.LOG_TAG, "splashView hide");
					splashView.setVisibility(View.GONE);
					// next two calls can render views into textures
					// we can call it only after GLSurfaceView.onResume
					//glView.bringToFront();
					JNITextField.ShowVisibleTextFields();
					JNIWebView.ShowVisibleWebViews();
				}
			}
		});
		
		glView.SetMultitouchEnabled(nativeIsMultitouchEnabled());
	}
}

