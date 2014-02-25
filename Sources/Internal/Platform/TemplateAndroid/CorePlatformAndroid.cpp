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


#include "Platform/TemplateAndroid/CorePlatformAndroid.h"

//#include "Core/Core.h"

#if defined(__DAVAENGINE_ANDROID__)

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

#include "Platform/Thread.h"
#include "Platform/DeviceInfo.h"
#include "Input/InputSystem.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
	AndroidSystemDelegate::AndroidSystemDelegate(JavaVM *vm)
	{
		Logger::Debug("AndroidSystemDelegate::AndroidSystemDelegate()");

		this->vm = vm;
		environment = NULL;
		if (vm->GetEnv((void**)&environment, JNI_VERSION_1_4) != JNI_OK)
		{
			Logger::Debug("Failed to get the environment using GetEnv()");
		}
	}

	Core::eDeviceFamily Core::GetDeviceFamily()
	{
		float32 width = GetPhysicalScreenWidth();
		float32 height = GetPhysicalScreenHeight();
		float32 dpi = GetScreenDPI();

		float32 inches = sqrt((width * width) + (height * height)) / dpi;

		if (inches > 6.f)
			return DEVICE_PAD;

		return DEVICE_HANDSET;
	}

	CorePlatformAndroid::CorePlatformAndroid()
	: Core()
	{
		wasCreated = false;
		renderIsActive = false;
		width = 0;
		height = 0;
		screenOrientation = Core::SCREEN_ORIENTATION_PORTRAIT; //no need rotate GL for Android

		foreground = false;
	}

	int Core::Run(int argc, char * argv[], AppHandle handle)
	{
// 		CoreWin32Platform * core = new CoreWin32Platform();
// 		core->CreateWin32Window(handle);
		// 		core->Run();
// 		core->ReleaseSingletons();
// #ifdef ENABLE_MEMORY_MANAGER
// 		if (DAVA::MemoryManager::Instance() != 0)
// 		{
// 			DAVA::MemoryManager::Instance()->FinalLog();
// 		}
// #endif
		return 0;

	}

	void CorePlatformAndroid::Quit()
	{
		Logger::Debug("[CorePlatformAndroid::Quit]");
		QuitAction();
		Core::Quit();
	}

	void CorePlatformAndroid::QuitAction()
	{
		Logger::Debug("[CorePlatformAndroid::QuitAction]");

		if(Core::Instance())
		{
			Core::Instance()->SystemAppFinished();
		}

		FrameworkWillTerminate();

#ifdef ENABLE_MEMORY_MANAGER
		if (DAVA::MemoryManager::Instance() != 0)
		{
			DAVA::MemoryManager::Instance()->FinalLog();
		}
#endif

		Logger::Debug("[CorePlatformAndroid::QuitAction] done");
	}

	void CorePlatformAndroid::RepaintView()
	{
		if(renderIsActive)
		{
			DAVA::RenderManager::Instance()->Lock();
			Core::SystemProcessFrame();
			DAVA::RenderManager::Instance()->Unlock();
		}
	}

	void CorePlatformAndroid::ResizeView(int32 w, int32 h)
	{
		if(width != w || height != h)
		{
			width = w;
			height = h;

			UpdateScreenMode();
		}
	}

	void CorePlatformAndroid::UpdateScreenMode()
	{
		Logger::Debug("[CorePlatformAndroid::UpdateScreenMode] start");
		UIControlSystem::Instance()->SetInputScreenAreaSize(width, height);
		Core::Instance()->SetPhysicalScreenSize(width, height);

		RenderManager::Instance()->InitFBSize(width, height);
        RenderManager::Instance()->Init(width, height);

		Logger::Debug("[CorePlatformAndroid::] w = %d, h = %d", width, height);
		Logger::Debug("[CorePlatformAndroid::UpdateScreenMode] done");
	}

	void CorePlatformAndroid::CreateAndroidWindow(const char8 *docPath, const char8 *assets, const char8 *logTag, AndroidSystemDelegate * sysDelegate)
	{
		androidDelegate = sysDelegate;
		externalStorage = docPath;

		Core::CreateSingletons();

		Logger::SetTag(logTag);
	}

	void CorePlatformAndroid::RenderRecreated(int32 w, int32 h)
	{
		Logger::Debug("[CorePlatformAndroid::RenderRecreated] start");

		renderIsActive = true;

		Thread::InitGLThread();

		if(wasCreated)
		{
			RenderManager::Instance()->Lost();
			RenderResource::SaveAllResourcesToSystemMem();
			RenderResource::LostAllResources();

			ResizeView(w, h);

			RenderManager::Instance()->Invalidate();
			RenderResource::InvalidateAllResources();
		}
		else
		{
			wasCreated = true;

			Logger::Debug("[CorePlatformAndroid::] before create renderer");
			RenderManager::Create(Core::RENDERER_OPENGL_ES_2_0);
			FileSystem::Init();

			RenderManager::Instance()->InitFBO(androidDelegate->RenderBuffer(), androidDelegate->FrameBuffer());
			Logger::Debug("[CorePlatformAndroid::] after create renderer");

			ResizeView(w, h);
			// DF-2274 - Set proper width and height before call FrameworkDidlaunched
			DeviceInfo::SetScreenInfo(w, h, 1);
			FrameworkDidLaunched();

			RenderManager::Instance()->SetFPS(60);

			//////////////////////////////////////////////////////////////////////////
			Core::Instance()->SystemAppStarted();

			StartForeground();
		}

		Logger::Debug("[CorePlatformAndroid::RenderRecreated] end");
	}

	void CorePlatformAndroid::OnCreateActivity()
	{
//		Logger::Debug("[CorePlatformAndroid::OnCreateActivity]");
	}

	void CorePlatformAndroid::OnDestroyActivity()
	{
//		Logger::Debug("[CorePlatformAndroid::OnDestroyActivity]");

		renderIsActive = false;
	}

	void CorePlatformAndroid::StartVisible()
	{
//		Logger::Debug("[CorePlatformAndroid::StartVisible]");
	}

	void CorePlatformAndroid::StopVisible()
	{
//		Logger::Debug("[CorePlatformAndroid::StopVisible]");
	}

	void CorePlatformAndroid::StartForeground()
	{
		Logger::Debug("[CorePlatformAndroid::StartForeground] start");

		if(wasCreated)
		{
			DAVA::ApplicationCore * core = DAVA::Core::Instance()->GetApplicationCore();
			if(core)
			{
				core->OnResume();
			}
			else
			{
				DAVA::Core::Instance()->SetIsActive(true);
			}
			DAVA::Core::Instance()->GoForeground();

			foreground = true;
		}
		Logger::Debug("[CorePlatformAndroid::StartForeground] end");
	}

	void CorePlatformAndroid::StopForeground(bool isLock)
	{
		Logger::Debug("[CorePlatformAndroid::StopForeground]");

		DAVA::ApplicationCore * core = DAVA::Core::Instance()->GetApplicationCore();
		if(core)
		{
			core->OnSuspend();
		}
		else
		{
			DAVA::Core::Instance()->SetIsActive(false);
		}
		DAVA::Core::Instance()->GoBackground(isLock);

		foreground = false;

		width = 0;
		height = 0;
	}

	void CorePlatformAndroid::KeyUp(int32 keyCode)
	{
		InputSystem::Instance()->GetKeyboard()->OnSystemKeyUnpressed(keyCode);
	}

	void CorePlatformAndroid::KeyDown(int32 keyCode)
	{
		InputSystem::Instance()->GetKeyboard()->OnSystemKeyPressed(keyCode);

		UIEvent * keyEvent = new UIEvent;
		keyEvent->keyChar = 0;
		keyEvent->phase = DAVA::UIEvent::PHASE_KEYCHAR;
		keyEvent->tapCount = 1;
		keyEvent->tid = InputSystem::Instance()->GetKeyboard()->GetDavaKeyForSystemKey(keyCode);

		InputSystem::Instance()->ProcessInputEvent(keyEvent);

		SafeDelete(keyEvent);
	}

	UIEvent CorePlatformAndroid::CreateInputEvent(int32 action, int32 id, float32 x, float32 y, float64 time, int32 source, int32 tapCount)
	{
		int32 phase = DAVA::UIEvent::PHASE_DRAG;
		switch(action)
		{
			case 5: //ACTION_POINTER_DOWN
			case 0://ACTION_DOWN
			phase = DAVA::UIEvent::PHASE_BEGAN;
			break;

			case 6://ACTION_POINTER_UP
			case 1://ACTION_UP
			phase = DAVA::UIEvent::PHASE_ENDED;
			break;

			case 2://ACTION_MOVE
			{
				if((source & 0x10) > 0)//SOURCE_CLASS_JOYSTICK
				{
					phase = DAVA::UIEvent::PHASE_JOYSTICK;
				}
				else //Touches
					phase = DAVA::UIEvent::PHASE_DRAG;
			}
			break;

			case 3://ACTION_CANCEL
			phase = DAVA::UIEvent::PHASE_CANCELLED;
			break;

			case 4://ACTION_OUTSIDE
			break;
		}

		UIEvent newEvent;
		newEvent.tid = id;
		newEvent.physPoint.x = x;
		newEvent.physPoint.y = y;
		newEvent.point.x = x;
		newEvent.point.y = y;
		newEvent.phase = phase;
		newEvent.tapCount = tapCount;
		newEvent.timestamp = time;

		return newEvent;
	}

	AAssetManager * CorePlatformAndroid::GetAssetManager()
	{
		return assetMngr;
	}

	void CorePlatformAndroid::SetAssetManager(AAssetManager * mngr)
	{
		assetMngr = mngr;
	}

	void CorePlatformAndroid::OnInput(int32 action, int32 id, float32 x, float32 y, float64 time, int32 source, int32 tapCount)
	{
		UIEvent touchEvent = CreateInputEvent(action, id, x, y, time, source, tapCount);

		if(touchEvent.phase == DAVA::UIEvent::PHASE_JOYSTICK)
		{
			InputSystem::Instance()->ProcessInputEvent(&touchEvent);
			return;
		}
		
		bool isFound = false;
		for(Vector<DAVA::UIEvent>::iterator it = totalTouches.begin(); it != totalTouches.end(); ++it)
		{
			if((*it).tid == touchEvent.tid)
			{
				(*it).physPoint.x = touchEvent.physPoint.x;
				(*it).physPoint.y = touchEvent.physPoint.y;
				(*it).phase = touchEvent.phase;
				(*it).tapCount = touchEvent.tapCount;

				isFound = true;
				break;
			}
		}
		if(!isFound)
		{
			totalTouches.push_back(touchEvent);
		}

		UIControlSystem::Instance()->OnInput(touchEvent.phase, totalTouches, totalTouches);

		for(Vector<DAVA::UIEvent>::iterator it = totalTouches.begin(); it != totalTouches.end(); )
		{
			if((DAVA::UIEvent::PHASE_ENDED == (*it).phase) || (DAVA::UIEvent::PHASE_CANCELLED == (*it).phase) || (DAVA::UIEvent::PHASE_JOYSTICK == (*it).phase))
			{
				it = totalTouches.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	bool CorePlatformAndroid::DownloadHttpFile(const String & url, const String & documentsPathname)
	{
		if(androidDelegate)
		{
			FilePath path(documentsPathname);
			return androidDelegate->DownloadHttpFile(url, path.GetAbsolutePathname());
		}

		return false;
	}

	AndroidSystemDelegate* CorePlatformAndroid::GetAndroidSystemDelegate() const
	{
		return androidDelegate;
	}
}
#endif // #if defined(__DAVAENGINE_ANDROID__)
