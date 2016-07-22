package com.dava.engine;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

public final class DavaActivity extends Activity
{
    public static String LOG_TAG = "DAVA";

    private DavaSurface surface;
    private ViewGroup layout;
    
    protected void onCreate(Bundle savedInstanceState)
    {
        Log.d(LOG_TAG, "DavaActivity.onCreate");
        super.onCreate(savedInstanceState);
        
        surface = new DavaSurface(getApplication(), this);
        
        layout = new RelativeLayout(this);
        layout.addView(surface);
        setContentView(layout);
    }

    protected void onStart()
    {
        Log.d(LOG_TAG, "DavaActivity.onStart");
        super.onStart();
    }

    protected void onResume()
    {
        Log.d(LOG_TAG, "DavaActivity.onResume");
        super.onResume();
    }

    protected void onPause()
    {
        Log.d(LOG_TAG, "DavaActivity.onPause");
        super.onPause();
    }

    protected void onStop()
    {
        Log.d(LOG_TAG, "DavaActivity.onStop");
        super.onStop();
    }

    protected void onRestart()
    {
        Log.d(LOG_TAG, "DavaActivity.onRestart");
        super.onRestart();
    }

    protected void onDestroy()
    {
        Log.d(LOG_TAG, "DavaActivity.onDestroy");
        super.onDestroy();
    }
    
    public void onWindowFocusChanged(boolean hasWindowFocus)
    {
        Log.d(LOG_TAG, String.format("DavaActivity.onWindowFocusChanged: focus=%b", hasWindowFocus));
    }
    
    public void onWindowVisibilityChanged(int visibility)
    {
        Log.d(LOG_TAG, String.format("DavaActivity.onWindowFocusChanged: visibility=%X", visibility));
    }
}
