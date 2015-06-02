package com.dava.framework;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.view.WindowManager;
import android.util.Log;
import android.content.ActivityNotFoundException;
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

	protected static void keepScreenOnOnResume() {
		if (!isEnabledSleepTimer)
			JNIActivity.GetActivity().getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
	}

	public static void OpenURL(final String url) {
		final JNIActivity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {

			@Override
			public void run() {
				try
				{
					final JNIActivity activity = JNIActivity.GetActivity();
					if (null == activity || activity.GetIsPausing()) {
						return;
					}

					Log.i(JNIConst.LOG_TAG, "[OpenURL] " + url);

					Intent exWeb = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
					activity.startActivity(exWeb);
				}
				catch(ActivityNotFoundException  e)
				{
					Log.i(JNIConst.LOG_TAG, "[OpenURL] failed with exeption: " + e.toString());
				}
			}
		});
	}

    public static String GenerateGUID()
    {
        return UUID.randomUUID().toString();
    }

}
