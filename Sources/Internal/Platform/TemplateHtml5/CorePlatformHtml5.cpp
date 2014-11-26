#include "Platform/TemplateHtml5/CorePlatformHtml5.h"

#if defined(__DAVAENGINE_HTML5__)
#include <emscripten/emscripten.h>

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

#define HTML5_WIDTH 960
#define HTML5_HEIGHT 640

namespace DAVA {
    static Vector<DAVA::UIEvent> activeTouches;
	bool CorePlatformHtml5::s_bMouseDown = false;
	int CorePlatformHtml5::s_xPos = 0;
	int CorePlatformHtml5::s_yPos = 0;


    void CorePlatformHtml5::Run()
    {
   		Core::Instance()->SetIsActive(true);
		Core::Instance()->SystemAppStarted();
		emscripten_set_main_loop(CorePlatformHtml5::EnterFrameRoutine, 0, true);
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
				case SDL_VIDEORESIZE:
					CorePlatformHtml5::ResizeEvent(event.resize);
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

	void CorePlatformHtml5::ResizeEvent(SDL_ResizeEvent ev)
	{
		((CorePlatformHtml5*)Core::Instance())->Resize((int32)ev.w, (int32)ev.h);
	}
	
	void CorePlatformHtml5::Resize(int32 nWidth, int32 nHeight)
	{
		RenderManager::Instance()->Init(nWidth, nHeight);
		UIControlSystem::Instance()->SetInputScreenAreaSize(nWidth, nHeight);
		
		Core::Instance()->UnregisterAllAvailableResourceSizes();
 		Core::Instance()->RegisterAvailableResourceSize(nWidth, nHeight, "Gfx");

 		Core::Instance()->SetPhysicalScreenSize(nWidth, nHeight);
		Core::Instance()->SetVirtualScreenSize(nWidth, nHeight);
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
       	if(ev.x == 0 && ev.y == 0)
       	{
       		if(s_bMouseDown == false)
       		{
       			s_xPos = 480;
       			s_yPos = 320;
				CorePlatformHtml5::ProcessMouseEvent(MOUSE_MOVE, s_xPos, s_yPos);
				CorePlatformHtml5::ProcessMouseEvent(MOUSE_DOWN, s_xPos, s_yPos);
				s_bMouseDown = true;
			}
			if( (s_xPos + ev.xrel) < 0 || (s_xPos + ev.xrel) > HTML5_WIDTH ||
				(s_yPos + ev.yrel) < 0 || (s_yPos + ev.yrel) > HTML5_HEIGHT )
			{
				CorePlatformHtml5::ProcessMouseEvent(MOUSE_UP, s_xPos, s_yPos);
       			s_xPos = 480;
       			s_yPos = 320;
       			CorePlatformHtml5::ProcessMouseEvent(MOUSE_MOVE, s_xPos, s_yPos);
				CorePlatformHtml5::ProcessMouseEvent(MOUSE_DOWN, s_xPos, s_yPos);
			}			
			s_xPos += ev.xrel;
			s_yPos += ev.yrel;
			CorePlatformHtml5::ProcessMouseEvent(MOUSE_MOVE, s_xPos, s_yPos, SDL_BUTTON_LMASK);
       	}
       	else
       	{
       		if(s_bMouseDown == true)
       		{
       			CorePlatformHtml5::ProcessMouseEvent(MOUSE_UP, s_xPos, s_yPos);
       			s_bMouseDown = false;
       		}
       		CorePlatformHtml5::ProcessMouseEvent(MOUSE_MOVE, ev.x, ev.y, ev.state);
       	}
    }
    
	void CorePlatformHtml5::MouseUpEvent(SDL_MouseButtonEvent ev)
	{
		if(s_bMouseDown == false )
		{
       		CorePlatformHtml5::ProcessMouseEvent(MOUSE_UP, ev.x, ev.y);
       	}
    }
    
	void CorePlatformHtml5::MouseDownEvent(SDL_MouseButtonEvent ev)
	{
		if(s_bMouseDown == false )
		{
			CorePlatformHtml5::ProcessMouseEvent(MOUSE_DOWN, ev.x, ev.y);
		}
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
    
    void CorePlatformHtml5::Init()
    {
    	SDL_Init(SDL_INIT_EVERYTHING);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 3);		
    	SDL_Surface *surf = SDL_SetVideoMode(0, 0, 32, SDL_OPENGL);
    	if(surf != NULL)
    	{
    		printf("GLES2 context created successfully\n");
    	}
    	else
    	{
    		printf("Error creating GLES2 context\n");
    	}
    	SDL_StartTextInput();
                              
	    RenderManager::Create(Core::RENDERER_OPENGL_ES_2_0);
		RenderManager::Instance()->Create();

		FrameworkDidLaunched();
		emscripten_set_canvas_size(HTML5_WIDTH, HTML5_HEIGHT);
		
		// Shouldn't be here, but won't work properly without this. Bug in EMSCRIPTEN?
		Resize(HTML5_WIDTH, HTML5_HEIGHT);
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