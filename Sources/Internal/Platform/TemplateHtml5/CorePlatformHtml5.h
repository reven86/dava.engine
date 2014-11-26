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
        void Resize(int32 nWidth, int32 nHeight);
        
        EGLDisplay m_eglDisplay;
        EGLContext m_eglContext;
        EGLSurface m_eglSurface;
        static bool s_bMouseDown;
        static int s_xPos;
		static int s_yPos;
        void EnterFrameRoutineInternal();
        static void EnterFrameRoutine();
        
  	    static void KeyDownEvent(SDL_KeyboardEvent ev);
		static void KeyUpEvent(SDL_KeyboardEvent ev);
		static void MouseMoveEvent(SDL_MouseMotionEvent ev);
		static void MouseUpEvent(SDL_MouseButtonEvent ev);
		static void MouseDownEvent(SDL_MouseButtonEvent ev);
		static void ResizeEvent(SDL_ResizeEvent ev);
		static void AppQuit();
		
		enum MouseMessage
        {
        	MOUSE_UP = 1,
        	MOUSE_DOWN = 2,
        	MOUSE_MOVE = 3,
        }; 

        static void ProcessMouseEvent(MouseMessage message, int x, int y, unsigned int state = 0);
        static int32 MoveTouchsToVector(MouseMessage message, int x, int y, Vector<UIEvent> *outTouchesm, unsigned int state = 0);
    };

};


#endif

#endif