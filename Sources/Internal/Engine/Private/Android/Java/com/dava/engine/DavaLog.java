package com.dava.engine;

import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;

public class DavaLog
{
    private static Logger logger;

    static
    {
        logger = Logger.getLogger("com.dava.engine.DavaLog");
        logger.setLevel(Level.FINEST);
    }

    public static Logger getLogger()
    {
        return logger;
    }

    public static int e(String tag, String msg)
    {
        logger.log(Level.SEVERE, tag + ": " + msg);
        return android.util.Log.e(tag, msg);
    }

    public static int w(String tag, String msg)
    {
        logger.log(Level.WARNING, tag + ": " + msg);
        return android.util.Log.w(tag, msg);
    }

    public static int i(String tag, String msg)
    {
        logger.log(Level.INFO, tag + ": " + msg);
        return android.util.Log.i(tag, msg);
    }

    public static int d(String tag, String msg)
    {
        logger.log(Level.FINEST, tag + ": " + msg);
        return android.util.Log.d(tag, msg);
    }
}