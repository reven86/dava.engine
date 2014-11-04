package com.dava.framework;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.view.WindowManager;
import java.util.UUID;

public class JNIUtils {
	private static boolean isEnabledSleepTimer = true;
	
	public static void DisableSleepTimer() {
		isEnabledSleepTimer = false;
		Activity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				JNIActivity.GetActivity().getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
			}
		});
	}
	
	public static void EnableSleepTimer() {
		isEnabledSleepTimer = true;
		Activity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				JNIActivity.GetActivity().getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
			}
		});
	}
	
	protected static void onResume() {
		if (!isEnabledSleepTimer)
			JNIActivity.GetActivity().getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
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

    public static String GenerateGUID()
    {
        return UUID.randomUUID().toString();
    }

}
