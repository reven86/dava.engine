package com.dava.framework;

import java.util.List;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ResolveInfo;
import android.content.pm.ServiceInfo;
import android.util.Log;

import com.bda.controller.Controller;
import com.bda.controller.IControllerService;

public class MogaFixForLollipop {
	
    public static void init( Controller controller, Context context )
    {
        boolean mIsBound = false;
        java.lang.reflect.Field fIsBound = null;
        android.content.ServiceConnection mServiceConnection = null;
        java.lang.reflect.Field fServiceConnection = null;
        try
        {
            Class<?> cMogaController = controller.getClass();
            fIsBound = cMogaController.getDeclaredField( "mIsBound" );
            fIsBound.setAccessible( true );
            mIsBound = fIsBound.getBoolean( controller );
            fServiceConnection = cMogaController.getDeclaredField( "mServiceConnection" );
            fServiceConnection.setAccessible( true );
            mServiceConnection = ( android.content.ServiceConnection ) fServiceConnection.get( controller );        
        }
        catch( NoSuchFieldException e )
        {
            Log.e( "SuperMogaHack", "Lollipop Hack NoSuchFieldException (get)", e );
        }
        catch( IllegalAccessException e )
        {
            Log.e( "SuperMogaHack", "Lollipop Hack IllegalAccessException (get)", e );
        }
        catch( IllegalArgumentException e )
        {
            Log.e( "SuperMogaHack", "Lollipop Hack IllegalArgumentException (get)", e );
        }
        if( ( !mIsBound ) && ( mServiceConnection != null ) )
        {
            // Convert implicit intent to explicit intent, see http://stackoverflow.com/a/26318757
            Intent intent = new Intent( IControllerService.class.getName() );
            List<ResolveInfo> resolveInfos = context.getPackageManager().queryIntentServices( intent, 0 );
            if( resolveInfos == null || resolveInfos.size() != 1 )
            {
                Log.e( "SuperMogaHack", "Somebody is trying to intercept our intent. Disabling MOGA controller for security." );
                return;
            }
            ServiceInfo serviceInfo = resolveInfos.get( 0 ).serviceInfo;
            String packageName = serviceInfo.packageName;
            String className = serviceInfo.name;
            intent.setComponent( new ComponentName( packageName, className ) );
            
            // Start the service explicitly
            context.startService( intent );
            context.bindService( intent, mServiceConnection, 1 );
            try
            {
                fIsBound.setBoolean( controller, true );
            }
            catch( IllegalAccessException e )
            {
                Log.e( "SuperMogaHack", "Lollipop Hack IllegalAccessException (set)", e );
            }
            catch( IllegalArgumentException e )
            {
                Log.e( "SuperMogaHack", "Lollipop Hack IllegalArgumentException (set)", e );
            }
        }
    }
}
