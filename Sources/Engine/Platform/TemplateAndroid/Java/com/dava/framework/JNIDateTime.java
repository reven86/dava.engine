package com.dava.framework;

import java.util.TimeZone;
import java.util.Date;
import java.util.Locale;

public class JNIDateTime {
	final static String TAG = "JNIDateTime";

	public static String GetTimeAsString(final String format,final String countryCode, long timeStamp, int timeZoneOffset)
	{
        TimeZone tz = TimeZone.getTimeZone("");
        tz.setRawOffset(timeZoneOffset*1000);
        Locale loc = new Locale(countryCode);      
        Date dt = new Date();
        dt.setTime(timeStamp*1000);
        Strftime formatter = new Strftime(format, loc);
        formatter.setTimeZone(tz);
        
        return formatter.format(dt);
	}
	
	public static int GetLocalTimeZoneOffset()
	{
		TimeZone tz = TimeZone.getDefault();
		Date now = new Date();
		int offsetFromUtc = tz.getOffset(now.getTime()) / 1000;
		return offsetFromUtc;
	}
}
