#if defined(__DAVAENGINE_HTML5__)
#include "Platform/Thread.h"
#include <emscripten/emscripten.h>

namespace DAVA
{

void* PthreadMain (void * param)
{
    Logger::Info("[PthreadMain] param = %p", param);
    
    Thread * t = (Thread*)param;
    
    /*if(t->needCopyContext)
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
    }*/
    
    t->state = Thread::STATE_RUNNING;
    t->msg(t);
    
    /*if(t->needCopyContext)
    {
        //TODO: Restore context
        bool ret = eglMakeCurrent(t->currentDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        Logger::Info("[PthreadMain] restore eglMakeCurrent returned = %d", ret);
    }*/
    
    t->state = Thread::STATE_ENDED;
    t->Release();
    
    //pthread_exit(0);
    return NULL;
}
void EmscriptenThreadMain(void *p)
{
	PthreadMain(p);
}
 
void Thread::StartHtml5()
{
    /*if(needCopyContext)
    {
        localContext = EGL_NO_CONTEXT;
        //		bool ret = eglMakeCurrent(currentDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        //		Logger::Info("[PthreadMain] restore eglMakeCurrent returned = %d", ret);
    }*/
    
    emscripten_async_call(EmscriptenThreadMain, (void*)this, 5000);
    //PthreadMain((void*)this);
    //pthread_t threadId;
    //pthread_create(&threadId, 0, PthreadMain, (void*)this);
    
    Logger::Info("[Thread::StartHtml5]");
}

Thread::ThreadId Thread::GetCurrentThreadId()
{
	ThreadId ret;
	ret.internalTid = 0;
	return ret;
}
    
bool Thread::IsMainThread()
{
    return 1;
}
};
#endif