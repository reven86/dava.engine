/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "Platform/Thread.h"

#if defined(__DAVAENGINE_ANDROID__)

#include <EGL/eglplatform.h>
#include <EGL/egl.h>

#include "Platform/TemplateAndroid/CorePlatformAndroid.h"

namespace DAVA
{

#include <unistd.h>

#include <pthread.h>

pid_t Thread::mainThreadId = 0;

EGLContext Thread::currentContext;
EGLDisplay Thread::currentDisplay;
EGLSurface Thread::currentDrawSurface;
EGLSurface Thread::currentReadSurface;



bool GetConfig(EGLDisplay disp, EGLConfig& bestConfig)
{
	EGLint count = 0;
	if (!eglGetConfigs(disp, NULL, 0, &count))
	{
		Logger::Error("[GetConfig] cannot query count of all configs");
		return false;
	}

	Logger::Info("[GetConfig] Count = %d", count);

	EGLConfig* configs = new EGLConfig[count];
	if (!eglGetConfigs(disp, configs, count, &count))
	{
		Logger::Error("[GetConfig] cannot query all configs");
		return false;
	}

	int bestMatch = 1<<30;
	int bestIndex = -1;

	int i;
	for (i = 0; i < count; i++)
	{
		int match = 0;
		EGLint surfaceType = 0;
		EGLint blueBits = 0;
		EGLint greenBits = 0;
		EGLint redBits = 0;
		EGLint alphaBits = 0;
		EGLint depthBits = 0;
		EGLint stencilBits = 0;
		EGLint renderableFlags = 0;

		EGLint configId = 0;

		eglGetConfigAttrib(disp, configs[i], EGL_SURFACE_TYPE, &surfaceType);
		eglGetConfigAttrib(disp, configs[i], EGL_BLUE_SIZE, &blueBits);
		eglGetConfigAttrib(disp, configs[i], EGL_GREEN_SIZE, &greenBits);
		eglGetConfigAttrib(disp, configs[i], EGL_RED_SIZE, &redBits);
		eglGetConfigAttrib(disp, configs[i], EGL_ALPHA_SIZE, &alphaBits);
		eglGetConfigAttrib(disp, configs[i], EGL_DEPTH_SIZE, &depthBits);
		eglGetConfigAttrib(disp, configs[i], EGL_STENCIL_SIZE, &stencilBits);
		eglGetConfigAttrib(disp, configs[i], EGL_RENDERABLE_TYPE, &renderableFlags);

		eglGetConfigAttrib(disp, configs[i], EGL_CONFIG_ID, &configId);


		Logger::Info("Config[%d]: R%dG%dB%dA%d D%dS%d Type=%04x Render=%04x id = %d",
			i, redBits, greenBits, blueBits, alphaBits, depthBits, stencilBits, surfaceType, renderableFlags, configId);

		if ((surfaceType & EGL_WINDOW_BIT) == 0)
			continue;
		if ((renderableFlags & EGL_OPENGL_ES2_BIT) == 0)
			continue;
		if ((depthBits < 16) || (stencilBits < 8))
			continue;
		if ((redBits < 8) || (greenBits < 8) || (blueBits < 8) || (alphaBits < 8))
			continue;

		int penalty = depthBits - 16;
		match += penalty * penalty;
		penalty = redBits - 8;
		match += penalty * penalty;
		penalty = greenBits - 8;
		match += penalty * penalty;
		penalty = blueBits - 8;
		match += penalty * penalty;
		penalty = alphaBits - 8;
		match += penalty * penalty;
		penalty = stencilBits;
		match += penalty * penalty;

		if ((match < bestMatch) || (bestIndex == -1))
		{
			bestMatch = match;
			bestIndex = i;
			Logger::Info("Config[%d] is the new best config", i, configs[i]);
		}
	}

	if (bestIndex < 0)
	{
		delete[] configs;
		return false;
	}

	bestConfig = configs[bestIndex];
	delete[] configs;

	return true;
}



void * PthreadMain (void * param)
{
	Logger::Info("[PthreadMain] param = %p", param);

	Thread * t = (Thread*)param;

	if(t->needCopyContext)
    {
    	EGLConfig localConfig;
    	bool ret = GetConfig(Thread::currentDisplay, localConfig);
		Logger::Info("[PthreadMain] GetConfig returned = %d", ret);

    	if(ret)
    	{
        	EGLint contextAttrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
        	t->localContext = eglCreateContext(t->currentDisplay, localConfig, t->currentContext, contextAttrs);
//        	t->localContext = eglCreateContext(t->currentDisplay, localConfig, EGL_NO_CONTEXT, contextAttrs);
    	}

    	if(t->localContext == EGL_NO_CONTEXT)
    	{
    		Logger::Error("[PthreadMain] Can't create local context");
    	}

    	GLint surfAttribs[] =
    	{
    			EGL_HEIGHT, 768,
    			EGL_WIDTH, 1024,
    			EGL_NONE
    	};

        
    	EGLSurface readSurface = eglCreatePbufferSurface(t->currentDisplay, localConfig, surfAttribs);
//    	EGLSurface drawSurface = eglCreatePbufferSurface(t->currentDisplay, localConfig, surfAttribs);

        //TODO: set context
//		bool ret2 = eglMakeCurrent(t->currentDisplay, t->currentDrawSurface, t->currentReadSurface, t->localContext);
		bool ret2 = eglMakeCurrent(t->currentDisplay, readSurface, readSurface, t->localContext);
		Logger::Info("[PthreadMain] set eglMakeCurrent returned = %d", ret2);
    }

	t->state = Thread::STATE_RUNNING;
	t->msg(t);

    if(t->needCopyContext)
	{
        //TODO: Restore context
		bool ret = eglMakeCurrent(t->currentDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		Logger::Info("[PthreadMain] restore eglMakeCurrent returned = %d", ret);
	}

	t->state = Thread::STATE_ENDED;
	t->Release();

	pthread_exit(0);
}



void Thread::StartAndroid()
{
    if(needCopyContext)
	{
    	localContext = EGL_NO_CONTEXT;
//		bool ret = eglMakeCurrent(currentDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
//		Logger::Info("[PthreadMain] restore eglMakeCurrent returned = %d", ret);
	}
    
    pthread_t threadId;
	pthread_create(&threadId, 0, PthreadMain, (void*)this);

    Logger::Info("[Thread::StartAndroid]");
}

bool Thread::IsMainThread()
{
    return (mainThreadId == gettid());
}

void Thread::InitMainThread()
{
    mainThreadId = gettid();

    currentContext = eglGetCurrentContext();
    if(currentContext == EGL_NO_CONTEXT)
    {
		Logger::Error("[Thread::InitMainThread] EGL_NO_CONTEXT");
    }

    currentDisplay = eglGetCurrentDisplay();
	if (currentDisplay == EGL_NO_DISPLAY)
	{
		Logger::Error("[Thread::InitMainThread] EGL_NO_DISPLAY");
	}

    currentDrawSurface = eglGetCurrentSurface(EGL_DRAW);
    if(currentDrawSurface == EGL_NO_SURFACE)
    {
		Logger::Error("[Thread::InitMainThread] EGL_NO_SURFACE | EGL_DRAW");
    }

    currentReadSurface = eglGetCurrentSurface(EGL_READ);
    if(currentReadSurface == EGL_NO_SURFACE)
    {
		Logger::Error("[Thread::InitMainThread] EGL_NO_SURFACE | EGL_READ");
    }


    Logger::Info("[Thread::InitMainThread] %ld", mainThreadId);
}

void Thread::YieldThread()
{
    sched_yield();
}


};

#endif //#if defined(__DAVAENGINE_ANDROID__)

