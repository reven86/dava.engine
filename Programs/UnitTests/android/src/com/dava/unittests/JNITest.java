package com.dava.unittests;

import android.util.Log;

import com.dava.framework.JNIActivity;
import com.dava.framework.JNIConst;

public class JNITest {

	public static boolean PassString(String string) {

		return true;
	}
	
	public static int PassStringArray(String[] strings) {

		int counter = 0;
		for(int i = 0; i < strings.length; i++)
		{
			counter = i+1;
		}
		return counter;
	}
	
	public static Object GetObject() {

		JNITestObject p = new JNITestObject();
		return p;
	}
	
	public static void AskForCallsFromJava(int countJava, int countC, boolean releaseRef) {
		
		JNIActivity activity = UnitTests.GetActivity();

		if (activity != null)
		{
			final int cj = countJava;
			final int cc = countC;
			final boolean releaseLocalRef = releaseRef;
			activity.RunOnMainLoopThread(new Runnable() {
				@Override
				public void run() {
					Log.d(JNIConst.LOG_TAG, "[JNITest::AskForCallsFromJava]" + "JavaCalls = " + cj + "nativeCalls = " + cc );
					UnitTests activity = (UnitTests) UnitTests.GetActivity();
					activity.TestCallToNativeInitiatedByJava(cj, cc, releaseLocalRef);				
				}
			});
		}

	}
	
}
