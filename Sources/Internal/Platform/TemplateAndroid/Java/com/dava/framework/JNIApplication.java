package com.dava.framework;

import java.util.Locale;

import android.app.Application;
import android.content.pm.ApplicationInfo;
import android.content.res.Configuration;
import android.util.Log;

public class JNIApplication extends Application
{
	static JNIApplication app;
	
	private native void OnCreateApplication(String externalDocumentPath, 
			String internalExternalDocumentsPath, String appPath, 
			String logTag, String packageName, String commandLineParams); 
	private native void OnConfigurationChanged(); 
	private native void OnLowMemoryWarning(); 
	private native void OnTerminate(); 
	
	private String externalDocumentsDir;
	private String internalDocumentsDir;
	private Locale launchLocale;
	private boolean firstLaunch = true;
	private final String TAG = "JNIApplication";

	/**
	 * Initialize native framework core in first time.
	 * Should be called on activity starting.
	 */
	public void InitFramework(final String commandLineParams)
	{
	    if (firstLaunch) {
            ApplicationInfo info = getApplicationInfo();
            Log.w(JNIConst.LOG_TAG, String.format("[Application::InitFramework] Create Application. apkFilePath is %s", info.publicSourceDir)); 
            OnCreateApplication(externalDocumentsDir, internalDocumentsDir, 
            		info.publicSourceDir, JNIConst.LOG_TAG, 
            		info.packageName, commandLineParams);
            firstLaunch = false;
        }
	}

	@Override
	public void onCreate()
	{
		app = this;
		super.onCreate();
		Log.i(JNIConst.LOG_TAG, "[Application::onCreate] start"); 
        
		/*
		 * Core initialization moved to JNIActivity
		 */
	    JNINotificationProvider.Init();
        
	    if(null != this.getExternalFilesDir(null)) {
            externalDocumentsDir = this.getExternalFilesDir(null).getAbsolutePath() + "/"; 
        } else {
            externalDocumentsDir = "";
        }
        
        internalDocumentsDir = this.getFilesDir().getAbsolutePath() + "/";
        launchLocale = Locale.getDefault();
		
		Log.i(JNIConst.LOG_TAG, "[Application::onCreate] finish"); 
	}
	
	@Override
	public void onConfigurationChanged(Configuration newConfig)
	{
		Log.i(JNIConst.LOG_TAG, "[Application::onConfigurationChanged]");
		super.onConfigurationChanged(newConfig);
		// useless call to native empty method...
		OnConfigurationChanged();

		if (IsApplicationShouldBeRestarted())
		{
			Log.w(JNIConst.LOG_TAG, "[Application::onConfigurationChanged] Application should now be closed");
			System.exit(0);
		}
	}

	@Override
	public void onLowMemory()
	{
		Log.w(JNIConst.LOG_TAG, "[Application::onLowMemory]"); 

		OnLowMemoryWarning();

		super.onLowMemory(); 
	}
	
	@Override
	public void onTerminate()
	{
    	Log.w(JNIConst.LOG_TAG, "[Application::onTerminate]"); 

/*    	This method is for use in emulated process environments. 
 * 		It will never be called on a production Android device, 
 * 		where processes are removed by simply killing them; 
 * 		no user code (including this callback) is executed when doing so.
 */

		super.onTerminate();
	}
	
	public static JNIApplication GetApplication()
	{
		return app;
	}
	
	public String GetDocumentPath()
	{
		return externalDocumentsDir;
	}

	private boolean IsApplicationShouldBeRestarted()
	{
		if (!launchLocale.equals(Locale.getDefault()))
		{
			return true;
		}

		return false;
	}
	
	static {
		System.loadLibrary("gnustl_shared");
		System.loadLibrary("fmodex");
		System.loadLibrary("fmodevent");
	}
}

