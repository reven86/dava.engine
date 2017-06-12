package com.dava.engine;

import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;

public class DavaLog
{
    private static Logger logger;

    static
    {
        logger = Logger.getLogger("com.dava.engine.Log");
        logger.setLevel(Level.FINEST);
    }

    public static void addHandler(Handler handler)
    {
        logger.addHandler(handler);
    }

    public static int e(String tag, String msg)
    {
        android.util.Log.e(tag, msg);
        logger.log(Level.SEVERE, tag + ": " + msg);
        return 0;
    }

    public static int w(String tag, String msg)
    {
        android.util.Log.w(tag, msg);
        logger.log(Level.WARNING, tag + ": " + msg);
        return 0;
    }

    public static int i(String tag, String msg)
    {
        android.util.Log.i(tag, msg);
        logger.log(Level.INFO, tag + ": " + msg);
        return 0;
    }

    public static int d(String tag, String msg)
    {
        android.util.Log.d(tag, msg);
        logger.log(Level.FINEST, tag + ": " + msg);
        return 0;
    }
}