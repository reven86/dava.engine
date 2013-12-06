#include "Platform/TemplateHtml5/CorePlatformHtml5.h"

#if defined(__DAVAENGINE_HTML5__)
#include <emscripten/emscripten.h>

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

namespace DAVA {
    static Vector<DAVA::UIEvent> activeTouches;

    void CorePlatformHtml5::Run()
    {
   		Core::Instance()->SetIsActive(true);
		Core::Instance()->SystemAppStarted();
		emscripten_set_main_loop(CorePlatformHtml5::EnterFrameRoutine, 60, true);
    }
    
    void CorePlatformHtml5::EnterFrameRoutine()
    {
       ((CorePlatformHtml5*)Core::Instance())->EnterFrameRoutineInternal();
    }
    
	void CorePlatformHtml5::EnterFrameRoutineInternal()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_KEYDOWN:
					CorePlatformHtml5::KeyDownEvent(event.key);
					break;
				case SDL_KEYUP:
					CorePlatformHtml5::KeyUpEvent(event.key);
					break;
				case SDL_MOUSEBUTTONDOWN:
					CorePlatformHtml5::MouseDownEvent(event.button);
					break;
				case SDL_MOUSEBUTTONUP:
					CorePlatformHtml5::MouseUpEvent(event.button);
					break;
				case SDL_MOUSEMOTION:
					CorePlatformHtml5::MouseMoveEvent(event.motion);
					break;
				case SDL_QUIT:
					CorePlatformHtml5::AppQuit();
					return;
			}
		}
		RenderManager::Instance()->Lock();
		Core::Instance()->SystemProcessFrame();
		RenderManager::Instance()->Unlock();
	}   

	void CorePlatformHtml5::AppQuit()
	{
		Core::Instance()->SystemAppFinished();
		FrameworkWillTerminate();
		Core::Instance()->ReleaseSingletons();
#ifdef ENABLE_MEMORY_MANAGER
		if (DAVA::MemoryManager::Instance() != 0)
		{
			DAVA::MemoryManager::Instance()->FinalLog();
		}
#endif
		EM_ASM(
				FS.syncfs(function (err) {
				if (err) console.error(err);
				});
			);
	}
	
	void CorePlatformHtml5::MouseMoveEvent(SDL_MouseMotionEvent ev)
	{
      	//printf("Mouse move %d %d %d\n", ev.x, ev.y, ev.state);
       	CorePlatformHtml5::ProcessMouseEvent(MOUSE_MOVE, ev.x, ev.y, ev.state);
    }
    
	void CorePlatformHtml5::MouseUpEvent(SDL_MouseButtonEvent ev)
	{
		//printf("Mouse up %d %d\n", ev.x, ev.y);
       	CorePlatformHtml5::ProcessMouseEvent(MOUSE_UP, ev.x, ev.y);
    }
    
	void CorePlatformHtml5::MouseDownEvent(SDL_MouseButtonEvent ev)
	{
		//printf("Mouse down %d %d\n", ev.x, ev.y);
		CorePlatformHtml5::ProcessMouseEvent(MOUSE_DOWN, ev.x, ev.y);
    }
    
    void CorePlatformHtml5::ProcessMouseEvent(MouseMessage message, int x, int y, unsigned int state/*0*/)
    {
	    Vector<DAVA::UIEvent> touches;
		Vector<DAVA::UIEvent> emptyTouches;

		int32 touchPhase = CorePlatformHtml5::MoveTouchsToVector(message, x, y, &touches, state);
		UIControlSystem::Instance()->OnInput(touchPhase, emptyTouches, touches);
    }
    
    int32 CorePlatformHtml5::MoveTouchsToVector(MouseMessage message, int x, int y, Vector<UIEvent> *outTouches, unsigned int state/*0*/)
	{
		int button = 1;// For now only left mouse button processed
		int phase = UIEvent::PHASE_MOVE;
		if(message == MOUSE_DOWN)
		{
			phase = UIEvent::PHASE_BEGAN;
		}
		else if(message == MOUSE_UP)
		{
			phase = UIEvent::PHASE_ENDED;
		}
		else if(state & SDL_BUTTON_LMASK)
		{
			phase = UIEvent::PHASE_DRAG;
		}

		if(phase == UIEvent::PHASE_DRAG)
		{
			for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
			{
                it->physPoint.x = (float32)x;
                it->physPoint.y = (float32)y;
				it->phase = phase;
			}
		}

		bool isFind = false;
		for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
		{
			if(it->tid == button)
			{
				isFind = true;

				it->physPoint.x = (float32)x;
				it->physPoint.y = (float32)y;
				it->phase = phase;
				break;
			}
		}

		if(!isFind)
		{
			UIEvent newTouch;
			newTouch.tid = button;
			newTouch.physPoint.x = (float32)x;
			newTouch.physPoint.y = (float32)y;
			newTouch.phase = phase;
			activeTouches.push_back(newTouch);
		}

		for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
		{
			outTouches->push_back(*it);
		}

		if(phase == UIEvent::PHASE_ENDED || phase == UIEvent::PHASE_MOVE)
		{
			for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
			{
				if(it->tid == button)
				{
					activeTouches.erase(it);
					break;
				}
			}
		}
		return phase;
	}	

    
    void CorePlatformHtml5::KeyDownEvent(SDL_KeyboardEvent keyEv)
    {       	
       	Vector<DAVA::UIEvent> touches;
		Vector<DAVA::UIEvent> emptyTouches;

		for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
		{
			touches.push_back(*it);
		}

		//printf("Key down %d %d %d\n", keyEv.keysym.unicode, keyEv.keysym.sym, keyEv.keysym.sym & ~SDLK_SCANCODE_MASK);
		DAVA::UIEvent ev;
		ev.keyChar = (keyEv.keysym.sym & SDLK_SCANCODE_MASK || (keyEv.keysym.sym < 32 || keyEv.keysym.sym > 126)) ? 0 : (char16)keyEv.keysym.unicode;//ke->charCode;
		ev.phase = DAVA::UIEvent::PHASE_KEYCHAR;
		ev.tapCount = 1;
		ev.tid = InputSystem::Instance()->GetKeyboard()->GetDavaKeyForSystemKey((int32)(keyEv.keysym.sym & ~SDLK_SCANCODE_MASK));//ke->keyCode);
		touches.push_back(ev);

		UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
		touches.pop_back();
		UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
		
		InputSystem::Instance()->GetKeyboard()->OnSystemKeyPressed((int32)(keyEv.keysym.sym & ~SDLK_SCANCODE_MASK));//ke->keyCode);
    }
    
	void CorePlatformHtml5::KeyUpEvent(SDL_KeyboardEvent keyEv)
	{
       	InputSystem::Instance()->GetKeyboard()->OnSystemKeyUnpressed((int32)(keyEv.keysym.sym & ~SDLK_SCANCODE_MASK));//ke->keyCode);
    }
    
    /*EGLBoolean CreateEGLContext( EGLDisplay &eglDisplay,
								EGLContext &eglContext, EGLSurface &eglSurface )
	{
	   EGLint numConfigs;
	   EGLint majorVersion;
	   EGLint minorVersion;
	   EGLDisplay display;
	   EGLContext context;
	   EGLSurface surface;
	   EGLConfig config;
	   EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
	   
	   EGLint attribList[] =
	   {
		   EGL_RED_SIZE,       8,
		   EGL_GREEN_SIZE,     8,
		   EGL_BLUE_SIZE,      8,
		   EGL_ALPHA_SIZE,     8,
   		   EGL_STENCIL_SIZE,   8,
		   EGL_DEPTH_SIZE,     24,
		   EGL_NONE
	   };

	   // Get Display
	   display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	   if ( display == EGL_NO_DISPLAY )
	   {
		  return EGL_FALSE;
	   }

	   // Initialize EGL
	   if ( !eglInitialize(display, &majorVersion, &minorVersion) )
	   {
		  return EGL_FALSE;
	   }

	   // Get configs
	   if ( !eglGetConfigs(display, NULL, 0, &numConfigs) )
	   {
		  return EGL_FALSE;
	   }

	   // Choose config
	   if ( !eglChooseConfig(display, attribList, &config, 1, &numConfigs) )
	   {
		  return EGL_FALSE;
	   }

	   // Create a surface
	   surface = eglCreateWindowSurface(display, config, NULL, NULL);
	   if ( surface == EGL_NO_SURFACE )
	   {
		  return EGL_FALSE;
	   }

	   // Create a GL context
	   context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs );
	   if ( context == EGL_NO_CONTEXT )
	   {
		  return EGL_FALSE;
	   }
	   
	   // Make the context current
	   if ( !eglMakeCurrent(display, surface, surface, context) )
	   {
		  return EGL_FALSE;
	   }
   
	   //eglDisplay = display;
	   //eglSurface = surface;
	   //eglContext = context;
	   return EGL_TRUE;
	}*/ 
        
    void CorePlatformHtml5::Init()
    {
    	SDL_Init(SDL_INIT_EVERYTHING);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 3);		
    	SDL_Surface *surf = SDL_SetVideoMode(0, 0, 32, SDL_OPENGL);
    	//EGLBoolean res = CreateEGLContext( m_eglDisplay, m_eglContext, m_eglSurface );
    	if(surf != NULL)
    	{
    		printf("GLES2 context created successfully\n");
    	}
    	else
    	{
    		printf("Error creating GLES2 context\n");
    	}
                              
	    RenderManager::Create(Core::RENDERER_OPENGL_ES_2_0);
		RenderManager::Instance()->Create();
		
		FrameworkDidLaunched();
		
		emscripten_set_canvas_size(1024, 768);
		RenderManager::Instance()->Init(1024, 768);
		UIControlSystem::Instance()->SetInputScreenAreaSize(1024, 768);
		Core::Instance()->SetPhysicalScreenSize(1024, 768);
    }
    
    int Core::Run(int argc, char * argv[], AppHandle handle)
    {
    	CorePlatformHtml5 *core = new CorePlatformHtml5();
    	core->CreateSingletons();    
    	core->Init();	
    	core->Run();
    }
};

#endif