#ifndef __DAVAENGINE_CORE_PLATFORM_NACL_H__
#define __DAVAENGINE_CORE_PLATFORM_NACL_H__

#include "DAVAEngine.h"

#if defined(__DAVAENGINE_NACL__)

#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/var.h"
#include "ppapi_simple/ps_main.h"
#include "ppapi_simple/ps_event.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/c/ppb_core.h"
#include "ppapi/cpp/graphics_3d.h"

#include "ppapi/c/pp_resource.h"

//#include "ppapi/c/ppb_fullscreen.h"
//#include "ppapi/c/ppb_graphics_2d.h"
//#include "ppapi/c/ppb_image_data.h"
#include <ppapi/c/ppb_input_event.h>
#include <ppapi/cpp/input_event.h>
#include "ppapi/c/ppb_view.h"

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();


namespace DAVA
{
    class CorePlatformNaCl : public Core
    {
    public:
    	void MountFS();
		void Run();
		void Init();
		static void MainLoop(void* core, int flags);
		static void InitThread(void* core, int flags);
		static void Swapped(void* core, int flags);
		void HandleEvent(PSEvent* ps_event);
		virtual void Quit();
		
		enum MouseMessage
        {
        	MOUSE_UP = 1,
        	MOUSE_DOWN = 2,
        	MOUSE_MOVE = 3,
        };
        static void ProcessMouseEvent(MouseMessage message, PSEvent* me);
        static int32 MoveTouchsToVector(MouseMessage message, PSEvent* me, Vector<UIEvent> *outTouches);

        int height;
		int width;
		
		bool toSwapBuffers;
		PP_Instance m_instance;
		PP_Resource m_context;
		PPB_Graphics3D* ppb_g3d_interface;
		PPB_Instance* ppb_instance_interface;
		PPB_Core* ppb_core_interface;

		PPB_View* m_pView;
		PPB_KeyboardInputEvent* m_pKeyboardInput;
		static PPB_InputEvent* m_pInputEvent;
		static PPB_MouseInputEvent* m_pMouseInput;
		PPB_TouchInputEvent* m_pTouchInput;
		int32 m_key_code;
		int32_t timeout;
		float32 prevMousePositionX;
   		float32 prevMousePositionY;
		
    };

};


#endif

#endif
