#include "Platform/TemplateNaCl/CorePlatformNaCl.h"


#ifdef __DAVAENGINE_NACL__
#include "ppapi/cpp/module.h"
#include "ppapi/c/ppb_fullscreen.h"
#include "pthread.h"
#include "nacl_io/nacl_io.h"
#include <sys/mount.h>
#include "ppapi/cpp/var.h"


#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

namespace DAVA {

static Vector<DAVA::UIEvent> activeTouches;
/*
void MyMessageHandler(const pp::Var& key,
                           const pp::Var& value,
                           void* user_data) {
       if(key.is_string()){
       		if(key.AsString() == "size"){
       			
       		}
       }
}
  
instance_->RegisterMessageHandler("foo", &MyMessageHandler, NULL);
*/

void CorePlatformNaCl::InitThread(void* core, int flags) {
	Logger::Debug("CorePlatformNaCl::InitThread");
	Thread::InitMainThread();
	//new SoundSystem();
//	Thread::chromeThreadId =  pthread_self();
}

void CorePlatformNaCl::Swapped(void* core, int flags) {
	((CorePlatformNaCl*)core)->toSwapBuffers = true;
}

void CorePlatformNaCl::MainLoop(void* core, int flags) {
//Logger::Debug("MainLoop");
		if(glGetCurrentContextPPAPI()==0){
			Logger::Debug("glSetCurrentContextPPAPI");
			glSetCurrentContextPPAPI(((CorePlatformNaCl*)core)->m_context);
		}
		PP_CompletionCallback cc = PP_MakeCompletionCallback( CorePlatformNaCl::Swapped,core );
		((CorePlatformNaCl*)core)->ppb_g3d_interface->SwapBuffers(((CorePlatformNaCl*)core)->m_context, cc);
}

void CorePlatformNaCl::Run()
{
	Logger::Debug("CorePlatformNacl::Run");
	RenderManager::Instance()->Init(width, height);
	UIControlSystem::Instance()->SetInputScreenAreaSize(width, height);
	Core::Instance()->SetPhysicalScreenSize(width, height);
	//Core::Instance()->SetVirtualScreenSize(width, height);
	Core::Instance()->SetIsActive(true);
	Core::Instance()->SystemAppStarted();
	toSwapBuffers = false;
	PP_CompletionCallback cc = PP_MakeCompletionCallback(MainLoop, (void*)this);
	ppb_core_interface->CallOnMainThread(0,cc,0);
}



void CorePlatformNaCl::Init(){
	RenderManager::Create(Core::RENDERER_OPENGL_ES_2_0);
	RenderManager::Instance()->Create();
	FrameworkDidLaunched();

	m_instance = PSGetInstanceId();
	m_pInputEvent = (PPB_InputEvent*) PSGetInterface(PPB_INPUT_EVENT_INTERFACE);
	PPB_GetInterface get_browser = pp::Module::Get()->get_browser_interface();
	ppb_g3d_interface = (PPB_Graphics3D*)get_browser(PPB_GRAPHICS_3D_INTERFACE);
    if (!glInitializePPAPI(get_browser)){
    	Logger::Debug("ERROR: CorePlatformNacl::Init glInitializePPAPI failed");
    }
    ppb_core_interface = (PPB_Core*)(get_browser(PPB_CORE_INTERFACE));
    int32_t attribs[] = {
		//PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 16,
		PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 24,
		PP_GRAPHICS3DATTRIB_STENCIL_SIZE, 8,
		PP_GRAPHICS3DATTRIB_SAMPLES, 0,
		PP_GRAPHICS3DATTRIB_SAMPLE_BUFFERS, 0,
		PP_GRAPHICS3DATTRIB_WIDTH, width,
		PP_GRAPHICS3DATTRIB_HEIGHT, height,
		PP_GRAPHICS3DATTRIB_NONE
    };

    m_context = ppb_g3d_interface->Create(m_instance, 0, attribs);
    if (m_context==0) {
        Logger::Debug("ERROR: CorePlatformNacl::Init Failed create context.\n");
        return;
    }
    ppb_instance_interface = (PPB_Instance*)get_browser(PPB_INSTANCE_INTERFACE);
    int32_t success =  ppb_instance_interface->BindGraphics(m_instance, m_context);
    if (success == PP_FALSE)
    {
        glSetCurrentContextPPAPI(0);
        Logger::Debug("Failed to set context.\n");
        return;
    }
    glSetCurrentContextPPAPI(m_context);
    
    glViewport(0,0, width,height);
  	glClearColor(0.0, 0.5, 0.5, 1);
	glClearDepthf(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Thread::InitGLThread();
	PP_CompletionCallback cc = PP_MakeCompletionCallback(InitThread, (void*)this);
	ppb_core_interface->CallOnMainThread(0,cc,0);

	m_pView = (PPB_View*)PSGetInterface(PPB_VIEW_INTERFACE);
    m_pInputEvent = (PPB_InputEvent*) PSGetInterface(PPB_INPUT_EVENT_INTERFACE);
    m_pKeyboardInput = (PPB_KeyboardInputEvent*)PSGetInterface(PPB_KEYBOARD_INPUT_EVENT_INTERFACE);
    m_pMouseInput = (PPB_MouseInputEvent*) PSGetInterface(PPB_MOUSE_INPUT_EVENT_INTERFACE);
    m_pTouchInput = (PPB_TouchInputEvent*) PSGetInterface(PPB_TOUCH_INPUT_EVENT_INTERFACE);
	PSEventSetFilter(PSE_ALL);
};

void CorePlatformNaCl::Quit()
{
	Logger::Debug("Quit\n");
}

void CorePlatformNaCl::ProcessMouseEvent(MouseMessage message, PSEvent* me)
   {
	    Vector<DAVA::UIEvent> touches;
		Vector<DAVA::UIEvent> emptyTouches;

		int32 touchPhase = MoveTouchsToVector(message, me, &touches);
		UIControlSystem::Instance()->OnInput(touchPhase, emptyTouches, touches);
   }

int32 CorePlatformNaCl::MoveTouchsToVector(MouseMessage message, PSEvent* me, Vector<UIEvent> *outTouches)
	{
		int button = 1;// For now only left mouse button processed
		int phase = UIEvent::PHASE_MOVE;
		/*PP_InputEvent_Modifier*/ uint32_t modifiers =	m_pInputEvent->GetModifiers(me->as_resource);
		struct PP_Point location;
		CorePlatformNaCl* core = (CorePlatformNaCl*)CorePlatformNaCl::Instance();
		if(!DAVA::InputSystem::Instance()->IsCursorPining())
		{
			location = m_pMouseInput->GetPosition(me->as_resource);
		} 
		else 
		{
			location = m_pMouseInput->GetMovement(me->as_resource);
			core->prevMousePositionX = core->prevMousePositionX + location.x;
			core->prevMousePositionY = core->prevMousePositionY + location.y;
			location.x = core->prevMousePositionX + core->width/2;
			location.y = core->prevMousePositionY + core->height/2;
		}

		if(message == MOUSE_DOWN)
		{
			phase = UIEvent::PHASE_BEGAN;
		}
		else if(message == MOUSE_UP)
		{
			phase = UIEvent::PHASE_ENDED;
		}
		else if(modifiers & PP_INPUTEVENT_MODIFIER_LEFTBUTTONDOWN)
		{
			phase = UIEvent::PHASE_DRAG;
		}

		if(phase == UIEvent::PHASE_DRAG)
		{
			for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
			{
                it->physPoint.x = (float32)location.x;
                it->physPoint.y = (float32)location.y;
  			    it->phase = phase;
			}
		}

		bool isFind = false;
		for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
		{
			if(it->tid == button)
			{
				isFind = true;

                it->physPoint.x = (float32)location.x;
                it->physPoint.y = (float32)location.y;
				it->phase = phase;
				break;
			}
		}

		if(!isFind)
		{
			UIEvent newTouch;
			newTouch.tid = button;
			newTouch.physPoint.x = (float32)location.x;
			newTouch.physPoint.y = (float32)location.y;
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


void CorePlatformNaCl::HandleEvent(PSEvent* event)
{
	switch(event->type) {
	    case PSE_INSTANCE_DIDCHANGEVIEW: {
	    	struct PP_Rect rect;
			PPB_View* naclView = (PPB_View*)PSGetInterface(PPB_VIEW_INTERFACE);
			naclView->GetRect(event->as_resource, &rect);
			bool f = naclView->IsFullscreen(event->as_resource);
			if(f){
				fprintf(stderr,"CorePlatformNaCl::HandleEvent PSE_INSTANCE_DIDCHANGEVIEW fullscreen");
			}
			break;
	    }
	    case PSE_INSTANCE_HANDLEMESSAGE:{
		    fprintf(stderr,"PSE_INSTANCE_HANDLEMESSAGE\n");
	    	/*if((event->as_var).type==PP_VARTYPE_STRING ){
	    		fprintf(stderr,pp::Var(event->as_var).AsString().c_str());
	    	}*/
	    	break;
	    }
	    case PSE_INSTANCE_DIDCHANGEFOCUS: {
	    	if(event->as_bool==true){
	    		this->timeout = 0;
		    //	Core::Instance()->SetIsActive(true);
			//	SoundSystem::Instance()->Resume();
	    	} else {
		    	this->timeout = 50;
		    	InputSystem::Instance()->SetCursorPining(false);
			//	SoundSystem::Instance()->Suspend();
			//	Core::Instance()->SetIsActive(false);
	    	}
	    	break;
	    }
	    case PSE_INSTANCE_HANDLEINPUT: {
            PP_InputEvent_Type type = m_pInputEvent->GetType(event->as_resource);
			switch(type) {
			  case PP_INPUTEVENT_TYPE_MOUSEDOWN: {
				  //struct PP_Point location = m_pMouseInput->GetPosition(event->as_resource);
				  //Logger::Debug("PP_INPUTEVENT_TYPE_MOUSEDOWN x=%d,y=%d",location.x, location.y);
				  CorePlatformNaCl::ProcessMouseEvent(MOUSE_DOWN, event);
				  break;
			  }

			  case PP_INPUTEVENT_TYPE_MOUSEMOVE: {
     			  CorePlatformNaCl::ProcessMouseEvent(MOUSE_MOVE, event);
				  break;
			  }

			  case PP_INPUTEVENT_TYPE_MOUSEUP:{
				  CorePlatformNaCl::ProcessMouseEvent(MOUSE_UP, event);
				  break;
			  }
			  case PP_INPUTEVENT_TYPE_KEYDOWN: {
				  pp::InputEvent key_event(event->as_resource);
     			  pp::KeyboardInputEvent key(key_event);
		          m_key_code =(int32)key.GetKeyCode();
		          //Logger::Debug("PP_INPUTEVENT_TYPE_KEYDOWN GetKeyCode=%d ",m_key_code);
		          //if(m_key_code==8){
					  Vector<DAVA::UIEvent> touches;
					  Vector<DAVA::UIEvent> emptyTouches;

					  for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
					  {
						  touches.push_back(*it);
					  }

					  DAVA::UIEvent ev;
					  ev.keyChar = 0;
					  ev.phase = DAVA::UIEvent::PHASE_KEYCHAR;
					  ev.tapCount = 1;
					  ev.tid = InputSystem::Instance()->GetKeyboard()->GetDavaKeyForSystemKey(m_key_code);
					  touches.push_back(ev);

					  UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
					  touches.pop_back();
					  UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);

					  InputSystem::Instance()->GetKeyboard()->OnSystemKeyPressed(m_key_code);
		         /* } else if ((m_key_code==13) &&( key.GetModifiers()==PP_INPUTEVENT_MODIFIER_ALTKEY)){
					  PPB_Fullscreen* pFullscreen = (PPB_Fullscreen*)PSGetInterface(PPB_FULLSCREEN_INTERFACE);
					  PP_Bool fullscreen = pFullscreen->IsFullscreen(PSGetInstanceId());
          			  pFullscreen->SetFullscreen(PSGetInstanceId(),
                                       fullscreen ? PP_FALSE : PP_TRUE);
					  if(fullscreen!=PP_TRUE) {

					  } else {
					  }
		          }*/
				  break;
			  }
			  case PP_INPUTEVENT_TYPE_KEYUP: {
				  pp::InputEvent key_event(event->as_resource);
     			  pp::KeyboardInputEvent key(key_event);
     			 // uint32_t key_code = key.GetKeyCode();
     			 // Logger::Debug("PP_INPUTEVENT_TYPE_KEYUP GetKeyCode=%d",(int32)key_code);
		          InputSystem::Instance()->GetKeyboard()->OnSystemKeyUnpressed((int32)key.GetKeyCode());
				  break;
			  }
			  case PP_INPUTEVENT_TYPE_CHAR:{
				  pp::InputEvent key_event(event->as_resource);
     			  pp::KeyboardInputEvent key(key_event);
     			 // uint32_t key_code = key.GetKeyCode();
				  //Logger::Debug("PP_INPUTEVENT_TYPE_CHAR GetKeyCode=%d txt=%s",(int32)key_code,key.GetCharacterText().AsString().c_str());
				  //fprintf(stderr, "%s",key.GetCharacterText().AsString().c_str());
				  Vector<DAVA::UIEvent> touches;
				  Vector<DAVA::UIEvent> emptyTouches;

				  for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
				  {
					  touches.push_back(*it);
				  }

				  DAVA::UIEvent ev;
				  //ev.keyChar = (char16)ke->charCode;
				  ev.keyChar = (char16)key.GetCharacterText().AsString().c_str()[0];
				  ev.phase = DAVA::UIEvent::PHASE_KEYCHAR;
				  ev.tapCount = 1;
				  ev.tid = InputSystem::Instance()->GetKeyboard()->GetDavaKeyForSystemKey(m_key_code);
				  touches.push_back(ev);

				  UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
				  touches.pop_back();
				  UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);

				  InputSystem::Instance()->GetKeyboard()->OnSystemKeyPressed(m_key_code);
			      break;
			  }
			  default:
				break;
			}
			/* case PSE_INSTANCE_HANDLEINPUT */

	    break;
	    }
	 default:
      break;   
	}
}
void CorePlatformNaCl::MountFS(){
	umount("/");
	umount("/DAVAProject");
	mount("",                                       
        "/DAVAProject",                            
        "html5fs",                                
        0,                                        
        "type=PERSISTENT,expected_size=1073741824");
  
    
//    mount("", "/Data", "memfs", 0, "");
    
    mount("/",       
        "/",  
        "httpfs", 
        0,        
        "");    
 
/*	uint64 prevTime = SystemTimer::Instance()->AbsoluteMS();
	File* directorylist = File::Create("/tmphttp/Startup/directorylist.txt", File::OPEN | File::READ);
	File* filelist = File::Create("/tmphttp/Startup/filelist.txt", File::OPEN | File::READ);
	if(directorylist){
		while(!directorylist->IsEof()){
			char tempBuf[1024];
			directorylist->ReadLine(tempBuf, 1024);
			String tempStr(tempBuf);
			FileSystem::Instance()->CreateDirectory("/Data"+tempStr,false);
		}
	} else {
		fprintf(stderr,"open directorylist failed\n");
	}
	fprintf(stderr,"directory created %dms\n",(SystemTimer::Instance()->AbsoluteMS()- prevTime));
	uint64 startFiles = SystemTimer::Instance()->AbsoluteMS();
	if(filelist){
		while(!filelist->IsEof()){
			char tempBuf[1024];
			filelist->ReadLine(tempBuf, 1024);
			String tempStr(tempBuf);
			prevTime = SystemTimer::Instance()->AbsoluteMS();
			FileSystem::Instance()->CopyFile( "/tmphttp/Data" +tempStr,"/Data" + tempStr);
			fprintf(stderr,tempStr.c_str());
			fprintf(stderr," time %dms\n",(SystemTimer::Instance()->AbsoluteMS()- prevTime));
			//usleep(10000);
		}
	}else {
		fprintf(stderr,"open filelist failed\n");
	}
	fprintf(stderr,"files copied %dms\n",(SystemTimer::Instance()->AbsoluteMS() - startFiles));
	sleep(3);*/
//---------------------------------------------------------------------------------------------------	

/*directorylist->Seek(0, File::SEEK_FROM_START);
filelist->Seek(0, File::SEEK_FROM_START);
	if(directorylist){
		while(!directorylist->IsEof()){
			char tempBuf[1024];
			directorylist->ReadLine(tempBuf, 1024);
			String tempStrD(tempBuf);
			FileSystem::Instance()->CreateDirectory("/tmpmem"+tempStrD,false);
		}
	} else {
		fprintf(stderr,"open directorylist failed\n");
	}
	fprintf(stderr,"directory created %dms\n",(SystemTimer::Instance()->AbsoluteMS()- prevTime));
	startFiles = SystemTimer::Instance()->AbsoluteMS();
	if(filelist){
		while(!filelist->IsEof()){
			char tempBuf[1024];
			filelist->ReadLine(tempBuf, 1024);
			String tempStr(tempBuf);
			prevTime = SystemTimer::Instance()->AbsoluteMS();
			FileSystem::Instance()->CopyFile( "/tmphtml5" +tempStr,"/tmpmem" + tempStr);
			fprintf(stderr,tempStr.c_str());
			fprintf(stderr," time %dms\n",(SystemTimer::Instance()->AbsoluteMS()- prevTime));
		}
	}else {
		fprintf(stderr,"open filelist failed\n");
	}
	fprintf(stderr,"files copied %dms\n",(SystemTimer::Instance()->AbsoluteMS() - startFiles));
	
*/	
	
	
/*    String line;
    directorylist.close();
    filelist.close();
    fprintf(stderr,"wtf?\n");
    while (directorylist.good()) {
       	getline (directorylist,line);
       	fprintf(stderr,line.c_str());
       	fprintf(stderr,"iter\n");
    //   	FileSystem::Instance()->CreateDirectory(line,false);
    }
    while(filelist.good()){
	    getline (directorylist,line);
	    fprintf(stderr,line.c_str());fprintf(stderr,"iter\n");
	  //  FileSystem::Instance()->CopyFile("/tmp" + line, line);
    }*/
	/*if(FileSystem::Instance()->CopyDirectory("/tmp/Startup/Data/", "/Data/")){
		fprintf(stderr,"Startup copied\n");
	} else {
		fprintf(stderr,"Startup not copied\n");
	}*/
	
	

}
int Core::Run(int argc, char * argv[], AppHandle handle)
{

	CorePlatformNaCl *core = new CorePlatformNaCl();
	PSEventSetFilter(PSE_INSTANCE_DIDCHANGEVIEW);
	while(1){
		PSEvent* ps_event;
		bool sizeIsSetted = false;
		while ((ps_event = PSEventTryAcquire()) != NULL) {
      		struct PP_Rect rect;
			PPB_View* naclView = (PPB_View*)PSGetInterface(PPB_VIEW_INTERFACE);
			naclView->GetRect(ps_event->as_resource, &rect);
			core->width = rect.size.width;
			core->height = rect.size.height;
			sizeIsSetted = true;
	        PSEventRelease(ps_event);
    	}
		if(sizeIsSetted){
			break;
		}
    }
	
	core->CreateSingletons();
	core->MountFS();
	core->Init();	
	core->Run();
	core->timeout = 0;
	core->prevMousePositionX = 0;
	core->prevMousePositionY = 0;
	while(1){
		PSEvent* ps_event;
		while ((ps_event = PSEventTryAcquire()) != NULL) {
      		core->HandleEvent(ps_event);
	        PSEventRelease(ps_event);
    	}
//    	Logger::Debug("eventLoop");
    	if(core->toSwapBuffers){
	    	//Logger::Debug("eventLoop SwapBuffers");
	    	core->toSwapBuffers = false;
	    	RenderManager::Instance()->Lock();
			Core::Instance()->SystemProcessFrame();
			RenderManager::Instance()->Unlock();
			PP_CompletionCallback cc = PP_MakeCompletionCallback(CorePlatformNaCl::MainLoop, (void*)core);
			core->ppb_core_interface->CallOnMainThread(core->timeout,cc,0);
 		}
 		usleep(1000);
    }
}

PPB_InputEvent* CorePlatformNaCl::m_pInputEvent = NULL;
PPB_MouseInputEvent* CorePlatformNaCl::m_pMouseInput = NULL;
}
#endif //__DAVAENGINE_NACL__
