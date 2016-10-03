package com.dava.engine;

import android.os.Handler;
import android.os.Message;
import android.util.Log;
class eHandlerCommand
{
    static final int QUIT = 1;
    static final int TRIGGER_PROCESS_EVENTS = 2;
}

final class DavaCommandHandler extends Handler
{
    public void sendCommand(int command, Object param)
    {
        Message msg = obtainMessage();
        msg.what = command;
        msg.obj = param;
        sendMessage(msg);
    }

    public void sendQuit()
    {
        sendCommand(eHandlerCommand.QUIT, null);
    }

    public void sendTriggerProcessEvents(DavaSurfaceView view)
    {
        sendCommand(eHandlerCommand.TRIGGER_PROCESS_EVENTS, view);
    }
    
    @Override
    public void handleMessage(Message msg)
    {
        switch (msg.what)
        {
        case eHandlerCommand.QUIT:
            if (DavaActivity.instance() != null)
            {
                DavaActivity.instance().finish();
            }
            break;
        case eHandlerCommand.TRIGGER_PROCESS_EVENTS:
            DavaSurfaceView view = (DavaSurfaceView)msg.obj;
            view.processEvents();
            break;
        default:
            super.handleMessage(msg);
            break;
        }
    }
}
