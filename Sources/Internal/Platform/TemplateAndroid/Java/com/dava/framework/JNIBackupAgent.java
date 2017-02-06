package com.dava.framework;

import com.dava.engine.DavaActivity;

import android.content.Context;
import android.app.Activity;
import android.app.backup.BackupAgentHelper;
import android.app.backup.BackupManager;
import android.app.backup.RestoreObserver;
import android.app.backup.SharedPreferencesBackupHelper;

public class JNIBackupAgent extends BackupAgentHelper {
	public void onCreate() {
        SharedPreferencesBackupHelper helper = new SharedPreferencesBackupHelper(this, JNISharedPreferences.GetName());
        addHelper(JNISharedPreferences.GetName(), helper);
    }
	
	public static void Backup()
	{
		BackupManager bm = new BackupManager(getContext());
        try {
            bm.dataChanged();
        } catch(Exception e) {
            e.printStackTrace();
        }
	}
	
	// You should pass observer if you want to restore settings menually
	public static void Restore(RestoreObserver observer)
	{
		BackupManager bm = new BackupManager(getContext());
        try {
            bm.requestRestore(observer);
        } catch(Exception e) {
            e.printStackTrace();
        }
	}

    private static Context getContext()
    {
        // TODO: Just use DavaActivity when CoreV1 is removed
        Activity activity = JNIActivity.GetActivity();
        if (activity == null)
        {
            activity = DavaActivity.instance();
        }

        return activity.getApplicationContext();
    }
}
