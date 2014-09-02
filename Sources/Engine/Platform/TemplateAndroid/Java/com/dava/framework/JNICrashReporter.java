package com.dava.framework;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;
import java.util.Vector;

import android.os.Build;

public class JNICrashReporter {
	private static final int JAVA_STACK_OFFSET;

	static {
		int i = 0;
		for (StackTraceElement element: Thread.currentThread().getStackTrace()) {
			if (element.getClassName().equals(JNICrashReporter.class.getName())) {
				break;
			}
			i++;
		}
		JAVA_STACK_OFFSET = i;
	}

	public static String GetReport() {
		SimpleDateFormat sdf = new SimpleDateFormat("yyyy/MM/dd_HH:mm:ss");
		String currentDateandTime = sdf.format(new Date());

		String report = new String();
		report += ("\nTime: " + currentDateandTime + "\n");
		report += ("DeveiceInfo:\n");
		report += ("MANUFACTURER: " + Build.MANUFACTURER + "\n");
		report += ("MODEL: " + Build.MODEL + "\n");
		report += ("OS: " + Build.VERSION.RELEASE + "\n");
		report += ("Locale: " + Locale.getDefault().getDisplayLanguage(Locale.US) + "\n");
		report += ("Country: " + JNIActivity.GetActivity().getResources().getConfiguration().locale.getCountry() + "\n");
		report += ("TimeZone: " + TimeZone.getDefault().getDisplayName(Locale.US) + "\n");
		report += ("\n");

		return report;
	}

	public static void ThrowJavaExpetion(String[] cls, String[] method, int[] fileLine) throws Exception {
		Thread currentThread = Thread.currentThread();
		Vector<StackTraceElement> newStackTrace = new Vector<StackTraceElement>();
		StackTraceElement[] curStackTrace = currentThread.getStackTrace();
		for (int i = 0; i < cls.length; ++i) {
			newStackTrace.add(new StackTraceElement(
					cls[i],
					method[i] + String.format(" ps:%s", Integer.toHexString(fileLine[i])),
					method[i],
					fileLine[i]));
		}
		for (int i = JAVA_STACK_OFFSET; i < curStackTrace.length; ++i) {
			newStackTrace.add(curStackTrace[i]);
		}
		Exception exception = new Exception(GetReport());
		exception.setStackTrace(newStackTrace.toArray(new StackTraceElement[newStackTrace.size()]));

		Thread.UncaughtExceptionHandler handler = Thread.currentThread()
				.getUncaughtExceptionHandler();
		handler.uncaughtException(Thread.currentThread(), exception);
	}
}
