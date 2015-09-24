package com.dava.framework;

import java.util.TimeZone;
import java.text.DateFormat;
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
        
        String result;        
        if (format.equals("%x"))
        {
        	DateFormat dateFormat = DateFormat.getDateInstance(DateFormat.SHORT, loc);
        	result = dateFormat.format(dt);
        } else if (format.equals("%X"))
        {
        	DateFormat dateFormat = DateFormat.getTimeInstance(DateFormat.MEDIUM, loc);
        	result = dateFormat.format(dt);
        } else
        {
        	// this is old invalid implementation leave it only temporarily, see DAVA::DateTime.AsWString(String format)
        	Strftime formatter = new Strftime(format, loc);
            formatter.setTimeZone(tz);
            result = formatter.format(dt);	
        }
        return result;
	}
	
	public static int GetLocalTimeZoneOffset()
	{
		TimeZone tz = TimeZone.getDefault();
		Date now = new Date();
		int offsetFromUtc = tz.getOffset(now.getTime()) / 1000;
		return offsetFromUtc;
	}
}
