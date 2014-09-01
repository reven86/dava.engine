package com.dava.framework;

import android.app.Application;
import android.content.pm.ApplicationInfo;
import android.content.res.Configuration;
import android.util.Log;
import com.dava.framework.JNINotificationProvider;

public class JNIApplication extends Application
{
	static JNIApplication app;
	
	private native void OnCreateApplication(String externalDocumentPath, String internalExternalDocumentsPath, String appPath, String logTag, String packageName); 
	private native void OnConfigurationChanged(); 
	private native void OnLowMemory(); 
	private native void OnTerminate(); 
	
	private String externalDocumentsDir;
	private String internalDocumentsDir; 
	
	@Override
	public void onCreate()
	{
		app = this;
		super.onCreate();
	
        JNINotificationProvider.Init();

		ApplicationInfo info = getApplicationInfo();
		
		Log.i(JNIConst.LOG_TAG, "[Application::onCreate] start"); 
		
		externalDocumentsDir = this.getExternalFilesDir(null).getAbsolutePath();
		internalDocumentsDir = this.getFilesDir().getAbsolutePath();
		
		Log.w(JNIConst.LOG_TAG, String.format("[Application::onCreate] apkFilePath is %s", info.publicSourceDir)); 
		OnCreateApplication(externalDocumentsDir, internalDocumentsDir, info.publicSourceDir, JNIConst.LOG_TAG, info.packageName);


		Log.i(JNIConst.LOG_TAG, "[Application::onCreate] finish"); 
	}

	@Override
	public void onConfigurationChanged(Configuration newConfig)
	{
		Log.w(JNIConst.LOG_TAG, String.format("[Application::onConfigurationChanged]")); 

		super.onConfigurationChanged(newConfig);

		OnConfigurationChanged();
	}

	@Override
	public void onLowMemory()
	{
		Log.w(JNIConst.LOG_TAG, String.format("[Application::onLowMemory]")); 

		OnLowMemory();

		super.onLowMemory(); 
	}
	
	@Override
	public void onTerminate()
	{
    	Log.w(JNIConst.LOG_TAG, String.format("[Application::onTerminate]")); 

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
	
	static {
		System.loadLibrary("iconv_android");
		System.loadLibrary("fmodex");
		System.loadLibrary("fmodevent");
	}
}

