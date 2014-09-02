/* ***** BEGIN LICENSE BLOCK *****
 *
 * THIS FILE IS PART OF THE MOZILLA NPAPI SDK BASIC PLUGIN SAMPLE
 * SOURCE CODE. USE, DISTRIBUTION AND REPRODUCTION OF THIS SOURCE
 * IS GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS 
 * SOURCE IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.
 *
 * THE MOZILLA NPAPI SDK BASIC PLUGIN SAMPLE SOURCE CODE IS
 * (C) COPYRIGHT 2008 by the Mozilla Corporation
 * http://www.mozilla.com/
 *
 * Contributors:
 *  Josh Aas <josh@mozilla.com>
 *
 * ***** END LICENSE BLOCK ***** */

/*
 * This sample plugin uses the Cocoa event model and the Core Graphics
 * drawing model.
 */

#include "NPAPIEntryPoint.h"

#import "NPAPIPluginMacOS.h"
#import "OpenGLView.h"

// Structure containing pointers to functions implemented by the browser.
static NPNetscapeFuncs* browser;

/* Symbol called once by the browser to initialize the plugin. */
extern "C" NPError NP_Initialize(NPNetscapeFuncs* browserFuncs)
{
	NSLog(@"NP_Initialize is called");
	// Save the browser function table.
	browser = browserFuncs;

	return NPERR_NO_ERROR;
}

/* Function called by the browser to get the plugin's function table. */
extern "C" NPError NP_GetEntryPoints(NPPluginFuncs* pluginFuncs)
{
	NSLog(@"NP_GetEntryPoints is called");
	// Check the size of the provided structure based on the offset of the
	// last member we need.
	if (pluginFuncs->size < (offsetof(NPPluginFuncs, setvalue) + sizeof(void*)))
	{
		return NPERR_INVALID_FUNCTABLE_ERROR;
	}

	pluginFuncs->newp = NPP_New;
	pluginFuncs->destroy = NPP_Destroy;
	pluginFuncs->setwindow = NPP_SetWindow;
	pluginFuncs->newstream = NPP_NewStream;
	pluginFuncs->destroystream = NPP_DestroyStream;
	pluginFuncs->asfile = NPP_StreamAsFile;
	pluginFuncs->writeready = NPP_WriteReady;
	pluginFuncs->write = (NPP_WriteProcPtr)NPP_Write;
	pluginFuncs->print = NPP_Print;
	pluginFuncs->event = NPP_HandleEvent;
	pluginFuncs->urlnotify = NPP_URLNotify;
	pluginFuncs->getvalue = NPP_GetValue;
	pluginFuncs->setvalue = NPP_SetValue;

	return NPERR_NO_ERROR;
}

/* Function called once by the browser to shut down the plugin. */
extern "C" void NP_Shutdown(void)
{
}

/* Called to create a new instance of the plugin. */
NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc, char* argn[], char* argv[], NPSavedData* saved)
{
	// Select the Core Animation model, since we need accelerated OpenGL.
	NPBool supportsCoreAnimation = false;
	if (browser->getvalue(instance, NPNVsupportsCoreAnimationBool, &supportsCoreAnimation) == NPERR_NO_ERROR && supportsCoreAnimation)
	{
		browser->setvalue(instance, NPPVpluginDrawingModel, (void*)NPDrawingModelCoreAnimation);
	}
	else
	{
		NSLog(@"CoreGraphics drawing model not supported, can't create a plugin instance.\n");
		return NPERR_INCOMPATIBLE_VERSION_ERROR;
	}

	// Select the Cocoa event model.
	NPBool supportsCocoaEvents = false;
	if (browser->getvalue(instance, NPNVsupportsCocoaBool, &supportsCocoaEvents) == NPERR_NO_ERROR && supportsCocoaEvents)
	{
		browser->setvalue(instance, NPPVpluginEventModel, (void*)NPEventModelCocoa);
	}
	else
	{
		NSLog(@"Cocoa event model not supported, can't create a plugin instance.\n");
		return NPERR_INCOMPATIBLE_VERSION_ERROR;
	}

	// Browser is OK - can create the plugin instance.
	NPAPIPluginMacOS* pluginMacOS = [[NPAPIPluginMacOS alloc] initWithNPP:instance];
	instance->pdata = pluginMacOS;
	
	NPError npNewError = [pluginMacOS npNew:pluginType npMode:mode npArgCount:argc
								 npArgNames:argn npArgValues:argv npSavedData:saved];
	if (npNewError != NPERR_NO_ERROR)
	{
		return npNewError;
	}

	return NPERR_NO_ERROR;
}

/* Called to destroy an instance of the plugin. */
NPError NPP_Destroy(NPP instance, NPSavedData** save)
{
	if (instance->pdata)
	{
		[(NPAPIPluginMacOS*)(instance->pdata) release];
		instance->pdata = NULL;
	}

	return NPERR_NO_ERROR;
}

/* Called to update a plugin instances's NPWindow. */
NPError NPP_SetWindow(NPP instance, NPWindow* window)
{
	NSLog(@"NPP_SetWindow is called");
	NPAPIPluginMacOS* currentInstance = (NPAPIPluginMacOS*)(instance->pdata);
	if (currentInstance)
	{
		return [currentInstance npSetWindow:window];
	}

	return NPERR_NO_ERROR;
}

NPError NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16_t* stype)
{
  *stype = NP_ASFILEONLY;
  return NPERR_NO_ERROR;
}

NPError NPP_DestroyStream(NPP instance, NPStream* stream, NPReason reason)
{
  return NPERR_NO_ERROR;
}

int32_t NPP_WriteReady(NPP instance, NPStream* stream)
{
  return 0;
}

int32_t NPP_Write(NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buffer)
{
  return 0;
}

void NPP_StreamAsFile(NPP instance, NPStream* stream, const char* fname)
{
}

void NPP_Print(NPP instance, NPPrint* platformPrint)
{
  
}

int16_t NPP_HandleEvent(NPP instance, void* event)
{
	NPCocoaEvent* cocoaEvent = (NPCocoaEvent*)event;
	if (cocoaEvent)
	{
		NPAPIPluginMacOS* currentInstance = (NPAPIPluginMacOS*)(instance->pdata);
		if (currentInstance)
		{
			return [currentInstance npHandleEvent:cocoaEvent];
		}
	}

	return 0;
}

void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{

}

NPError NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
	NPAPIPluginMacOS* currentInstance = (NPAPIPluginMacOS*)(instance->pdata);
	if (currentInstance)
	{
		return [currentInstance npGetValue:variable withParam:value];
	}
	
	return NPERR_GENERIC_ERROR;
}

NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
  return NPERR_GENERIC_ERROR;
}
