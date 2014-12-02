package com.dava.framework;

import android.app.AlarmManager;
import android.app.PendingIntent;
import java.util.Timer;
import java.util.TimerTask;

import org.fmod.FMODAudioDevice;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.hardware.SensorManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.app.NotificationCompat.Builder;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import com.bda.controller.Controller;

import java.util.Calendar;

public abstract class JNIActivity extends Activity implements JNIAccelerometer.JNIAccelerometerListener
{
	private static int errorState = 0;

	private JNIAccelerometer accelerometer = null;
	protected JNIGLSurfaceView glView = null;
	private View splashView = null;
	
	private FMODAudioDevice fmodDevice = new FMODAudioDevice();
	
	private Controller mController;
	
	private native void nativeOnCreate(boolean isFirstRun);
	private native void nativeOnStart();
	private native void nativeOnStop();
	private native void nativeOnDestroy();
	private native void nativeIsFinishing();
	private native void nativeOnAccelerometer(float x, float y, float z);
    
    private boolean isFirstRun = true;
    private static String commandLineParams = null;
    private static boolean isRenderDestroy = false;
    
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
        
        commandLineParams = initCommandLineParams();

        // Initialize native framework core         
        JNIApplication.GetApplication().InitFramework(commandLineParams);
        
        //JNINotificationProvider.AttachToActivity();
        
        if(null != savedInstanceState)
        {
        	isFirstRun = savedInstanceState.getBoolean("isFirstRun");
        }
        
    	// The activity is being created.
        Log.i(JNIConst.LOG_TAG, "[Activity::onCreate]");
        
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
        
        splashView = GetSplashView();
        
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
        
        JNINotificationProvider.AttachToActivity(this);
        
		Intent intent = getIntent();
		if (null != intent) {
			String uid = intent.getStringExtra("uid");
			if (uid != null) {
				JNINotificationProvider.NotificationPressed(uid);
			}
		}

        if (splashView != null && !isRenderDestroy)
			splashView.setVisibility(View.GONE);
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
		Log.i("DAVA", "command line params: " + commandLine);
		return commandLine;
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

		
		JNITextField.HideAllTextFields();
		
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
        if(null != glView && null == splashView) {
        	glView.onResume();
        }
        
        Log.i(JNIConst.LOG_TAG, "[Activity::onResume] finish");
        
        JNIUtils.onResume();
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
        
        // Destroy keyboard layout if its hasn't been destroyed by lost focus (samsung lock workaround)
        JNITextField.DestroyKeyboardLayout(getWindowManager());
        
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
    
    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
    	super.onWindowFocusChanged(hasFocus);
    	
    	isRenderDestroy = false;
    	if (hasFocus) {
    		new Timer().schedule(new TimerTask() {
				
				@Override
				public void run() {
					runOnUiThread(new Runnable() {
						public void run() {
							if (splashView != null) {
								glView.setVisibility(View.VISIBLE);
								JNIActivity.GetActivity().glView.onResume();
							}
						}
					});
				}
 	 		}, 300);
    	}
    	
    	if(hasFocus) {
    		JNITextField.InitializeKeyboardLayout(getWindowManager(), glView.getWindowToken());
			HideNavigationBar(getWindow().getDecorView());
    	} else {
    		JNITextField.DestroyKeyboardLayout(getWindowManager());
    	}
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
	
	protected void OnRenderDestroyed() {
		isRenderDestroy = true;
    	runOnUiThread(new Runnable() {
			
			@Override
			public void run() {
				if (splashView != null) {
					glView.setVisibility(View.GONE);
					splashView.setVisibility(View.VISIBLE);
				}
			}
		});
	}
	
	protected void OnFirstFrameAfterDraw() {
		runOnUiThread(new Runnable() {
			
			@Override
			public void run() {
				if (splashView != null) {
					splashView.setVisibility(View.GONE);
				}
			}
		});
	}
}

