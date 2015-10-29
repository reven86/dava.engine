package com.dava.testbed;

import android.os.Bundle;
import android.view.Menu;

import com.dava.framework.JNIActivity;
import com.dava.framework.JNISurfaceView;

public class TestBed extends JNIActivity {

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

	public JNISurfaceView FindSurfaceView() {
		setContentView(R.layout.activity_main);
		JNISurfaceView view = (JNISurfaceView) findViewById(R.id.view1);
		return view;
	}

}
