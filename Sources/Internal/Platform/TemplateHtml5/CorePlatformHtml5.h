#ifndef __DAVAENGINE_CORE_PLATFORM_HTML5_H__
#define __DAVAENGINE_CORE_PLATFORM_HTML5_H__

#include "DAVAEngine.h"

#if defined(__DAVAENGINE_HTML5__)
#include <EGL/egl.h>
#include <SDL/SDL.h>

namespace DAVA
{
    class CorePlatformHtml5 : public Core
    {
    public:
        void Run();
        void Init();
        
        EGLDisplay m_eglDisplay;
        EGLContext m_eglContext;
        EGLSurface m_eglSurface;
        void EnterFrameRoutineInternal();
        static void EnterFrameRoutine();
        
  	    static void KeyDownEvent(SDL_KeyboardEvent ev);
		static void KeyUpEvent(SDL_KeyboardEvent ev);
		static void MouseMoveEvent(SDL_MouseMotionEvent ev);
		static void MouseUpEvent(SDL_MouseButtonEvent ev);
		static void MouseDownEvent(SDL_MouseButtonEvent ev);
		static void AppQuit();
		
		enum MouseMessage
        {
        	MOUSE_UP = 1,
        	MOUSE_DOWN = 2,
        	MOUSE_MOVE = 3,
        }; 

        static void ProcessMouseEvent(MouseMessage message, int x, int y, unsigned int state = 0);
        static int32 MoveTouchsToVector(MouseMessage message, int x, int y, Vector<UIEvent> *outTouchesm, unsigned int state = 0);
/*        
        void ExitCoreInternal();
        void HandleFullScreenInternal();
        void ResizeInternal();

        
        
        static var ExitCore(void *arg, var as3Args);
        static var HandleFullScreen(void *arg, var as3Args);
        static var Resize(void *arg, var as3Args);  
		      		

        virtual void Quit();
        void OnQuitAction();
        
        static bool m_bFinished;
        
        AS3::ui::Function EnterFrameFunc;
        AS3::ui::Function ResizeFunc;
        AS3::ui::Function HandleFullScreenFunc;
        AS3::ui::Function KeyDownFunc;
        AS3::ui::Function KeyUpFunc;
        AS3::ui::Function MouseMoveFunc;
        AS3::ui::Function MouseDownFunc;
        AS3::ui::Function MouseUpFunc;
                        
        flash::display::Stage3D m_stage3d;
        flash::display::Stage m_stage;
        flash::display3D::Context3D m_ctx3d;*/
    };

};


#endif

#endif