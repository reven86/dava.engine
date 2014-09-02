package com.dava.unittests;

import android.app.PendingIntent;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.NotificationCompat.Builder;
import android.view.Menu;
import com.dava.framework.JNIActivity;
import com.dava.framework.JNIGLSurfaceView;
import com.dava.unittests.R;

public class UnitTests extends JNIActivity {

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}

	public JNIGLSurfaceView GetSurfaceView() {
		setContentView(R.layout.activity_main);
		JNIGLSurfaceView view = (JNIGLSurfaceView) findViewById(R.id.view1);
		return view;
	}
	
	@Override
	public void InitNotification(Builder builder) {
		Intent intent = new Intent(this, UnitTests.class);
		PendingIntent pIntent = PendingIntent.getActivity(this, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);
		builder.setContentIntent(pIntent);
		builder.setSmallIcon(R.drawable.ic_launcher);
	}
}
