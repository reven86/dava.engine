package com.dava.framework;

import android.app.Activity;
import android.view.WindowManager;

public class JNIUtils {
	public static void DisableSleepTimer() {
		Activity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				JNIActivity.GetActivity().getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
			}
		});
	}
	
	public static void EnableSleepTimer() {
		Activity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				JNIActivity.GetActivity().getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
			}
		});
	}
	
	public static boolean IsFile(String path) {
		return LocalFileDescriptor.IsFile(path);
	}
	
	public static boolean IsDirectory(String path) {
		return LocalFileDescriptor.IsDirectory(path);
	}
}
