package com.dava.framework;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.view.WindowManager;
import android.util.Log;
import android.content.ActivityNotFoundException;
import java.util.UUID;

public class JNIUtils {
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
