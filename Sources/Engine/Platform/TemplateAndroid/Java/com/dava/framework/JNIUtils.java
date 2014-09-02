package com.dava.framework;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
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
	
	public static void OpenURL(final String url) {
		Activity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			
			@Override
			public void run() {
				Intent exWeb = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
				JNIActivity.GetActivity().startActivity(exWeb);
			}
		});
	}
}
