package com.dava.engine;

import java.util.UUID;

import android.net.Uri;
import android.util.Log;
import android.app.Activity;
import android.view.WindowManager;
import android.content.Intent;
import android.content.ActivityNotFoundException;

public class Utils
{
	private Utils()
	{
		
	}
	
	public static void disableSleepTimer()
	{
		final Activity activity = DavaActivity.instance();
		activity.runOnUiThread(new Runnable()
		{
			@Override
			public void run() 
			{
				DavaActivity.instance().getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
			}
		});
	}
	
	public static void enableSleepTimer()
	{
		final Activity activity = DavaActivity.instance();
		activity.runOnUiThread(new Runnable()
		{
			@Override
			public void run()
			{
				DavaActivity.instance().getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
			}
		});
	}

	public static void openURL(final String url)
	{
		final Activity activity = DavaActivity.instance();
		activity.runOnUiThread(new Runnable()
		{
			@Override
			public void run()
			{
				try
				{
					final DavaActivity activity = DavaActivity.instance();
					if (null == activity || activity.isPaused())
					{
						return;
					}

					Log.i(DavaActivity.LOG_TAG, "[OpenURL] " + url);

					Intent exWeb = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
					activity.startActivity(exWeb);
				}
				catch(ActivityNotFoundException e)
				{
					Log.i(DavaActivity.LOG_TAG, "[OpenURL] failed with exeption: " + e.toString());
				}
			}
		});
	}

    public static String generateGUID()
    {
        return UUID.randomUUID().toString();
    }
}