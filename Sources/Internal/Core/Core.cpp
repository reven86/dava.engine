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

#include "DAVAClassRegistrator.h"

#include "FileSystem/FileSystem.h"
#include "Base/ObjectFactory.h"
#include "Core/ApplicationCore.h"
#include "Core/Core.h"
#include "Core/PerformanceSettings.h"
#include "Render/RenderManager.h"
#include "Platform/SystemTimer.h"
#include "Platform/Thread.h"
#include "UI/UIScreenManager.h"
#include "UI/UIControlSystem.h"
#include "Input/InputSystem.h"
#include "Debug/DVAssert.h"
#include "Render/2D/TextBlock.h"
#include "Debug/Replay.h"
#include "Sound/SoundSystem.h"
#include "Sound/SoundEvent.h"
#include "Input/InputSystem.h"
#include "Platform/DPIHelper.h"
#include "Base/AllocatorFactory.h"
#include "Render/2D/FTFont.h"
#include "Scene3D/SceneFile/VersionInfo.h"
#include "Render/Image/ImageSystem.h"

#if defined(__DAVAENGINE_IPHONE__)
#include "Input/AccelerometeriPhone.h"
#elif defined(__DAVAENGINE_ANDROID__)
#	include "Input/AccelerometerAndroid.h"
#endif //PLATFORMS

#ifdef __DAVAENGINE_AUTOTESTING__
#include "Autotesting/AutotestingSystem.h"
#endif

#ifdef __DAVAENGINE_NVIDIA_TEGRA_PROFILE__
#include <EGL/eglext.h>
#endif //__DAVAENGINE_NVIDIA_TEGRA_PROFILE__

namespace DAVA 
{

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	static bool useAutodetectContentScaleFactor = false;
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)

	float Core::virtualToPhysical = 0;
	float Core::physicalToVirtual = 0;
	Vector2 Core::drawOffset;

	static ApplicationCore * core = 0;

Core::Core()
{
	globalFrameIndex = 1;
	isActive = false;
	firstRun = true;
	isConsoleMode = false;
	options = new KeyedArchive();
	fixedProportions = true;
    
    desirableIndex = 0;

    EnableReloadResourceOnResize(true);
}

Core::~Core()
{
	SafeRelease(options);
	SafeRelease(core);
}


void Core::CreateSingletons()
{
#ifndef __DAVAENGINE_IPHONE__
//	if (!Core::Instance())
//	{
//		//Logger::Warning("[Core::Create] failed / something wrong with template or your platform code / contact framework developers");
//	}
#endif
    
    
    // check types size
	new Logger();
	new AllocatorFactory();
	new JobManager();
	new FileSystem();
    FilePath::InitializeBundleName();
	
	FileSystem::Instance()->SetDefaultDocumentsDirectory();
    FileSystem::Instance()->CreateDirectory(FileSystem::Instance()->GetCurrentDocumentsDirectory(), true);
	
    new SoundSystem();

	if (isConsoleMode)
	{
		/*
			Disable all debug initialization messages in console mode
		 */
		Logger::Instance()->SetLogLevel(Logger::LEVEL_INFO);
	}
//	Logger::FrameworkDebug("[Core::Create] successfull");
    
	new LocalizationSystem();

	new SystemTimer();
	new Random();
	new AnimationManager();
	new FontManager();
	new UIControlSystem();
	new InputSystem();
	new RenderHelper();
    new RenderLayerManager();
	new PerformanceSettings();
    new VersionInfo();
    new ImageSystem();
	
#if defined __DAVAENGINE_IPHONE__
	new AccelerometeriPhoneImpl();
#elif defined(__DAVAENGINE_ANDROID__)
	new AccelerometerAndroidImpl();
#endif //#if defined __DAVAENGINE_IPHONE__
	
	new UIScreenManager();

#ifdef __DAVAENGINE_AUTOTESTING__
    new AutotestingSystem();
#endif

#if defined(__DAVAENGINE_WIN32__)
	Thread::InitMainThread();
#endif
    
    RegisterDAVAClasses();
    
    CheckDataTypeSizes();
}

// We do not create RenderManager until we know which version of render manager we want to create
void Core::CreateRenderManager()
{
    eRenderer renderer = (eRenderer)options->GetInt32("renderer");
    
    RenderManager::Create(renderer);	
}
        
void Core::ReleaseSingletons()
{
	PerformanceSettings::Instance()->Release();
	RenderHelper::Instance()->Release();
	UIScreenManager::Instance()->Release();
	UIControlSystem::Instance()->Release();
	FontManager::Instance()->Release();
	AnimationManager::Instance()->Release();
	SystemTimer::Instance()->Release();
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	Accelerometer::Instance()->Release();
	//SoundSystem::Instance()->Release();
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	LocalizationSystem::Instance()->Release();
//	Logger::FrameworkDebug("[Core::Release] successfull");
    FileSystem::Instance()->Release();
    SoundSystem::Instance()->Release();
	Random::Instance()->Release();
	RenderLayerManager::Instance()->Release();
	RenderManager::Instance()->Release();
#ifdef __DAVAENGINE_AUTOTESTING__
    AutotestingSystem::Instance()->Release();
#endif

	InputSystem::Instance()->Release();
	JobManager::Instance()->Release();
    VersionInfo::Instance()->Release();
	AllocatorFactory::Instance()->Release();
	Logger::Instance()->Release();
    ImageSystem::Instance()->Release();
}

void Core::SetOptions(KeyedArchive * archiveOfOptions)
{
	SafeRelease(options);

	options = SafeRetain(archiveOfOptions);
#if defined(__DAVAENGINE_IPHONE__)
		useAutodetectContentScaleFactor = options->GetBool("iPhone_autodetectScreenScaleFactor", false);
#elif defined(__DAVAENGINE_ANDROID__)
		useAutodetectContentScaleFactor = options->GetBool("Android_autodetectScreenScaleFactor", false);
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)

#if !defined(__DAVAENGINE_ANDROID__)
	//YZ android platform always use SCREEN_ORIENTATION_PORTRAIT and rotate system view and don't rotate GL view  
	screenOrientation = options->GetInt32("orientation", SCREEN_ORIENTATION_PORTRAIT);
#endif
}
    
void Core::CheckDataTypeSizes()
{
    CheckType(int8(), 8, "int8");
    CheckType(uint8(), 8, "uint8");
    CheckType(int16(), 16, "int16");
    CheckType(uint16(), 16, "uint16");
    CheckType(int32(), 32, "int32");
    CheckType(uint32(), 32, "uint32");
}

template <class T> void Core::CheckType(T t, int32 expectedSize, const char * typeString)
{
    if ((sizeof(t) * 8) != expectedSize)
    {
        Logger::Error("Size of %s is incorrect. Expected size: %d. Platform size: %d", typeString, expectedSize, sizeof(t));
    }
}

KeyedArchive * Core::GetOptions()
{
	return options;
}
	
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
bool Core::IsAutodetectContentScaleFactor()
{
	return useAutodetectContentScaleFactor;
}
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)

	
float32 Core::GetVirtualToPhysicalFactor()
{
	return virtualToPhysical;
}
	
float32 Core::GetPhysicalToVirtualFactor()
{
	return physicalToVirtual;
}

Core::eScreenOrientation Core::GetScreenOrientation()
{
	return (Core::eScreenOrientation)screenOrientation;
}

void Core::CalculateScaleMultipliers()
{
	needTorecalculateMultipliers = false;		

	virtualScreenWidth = requestedVirtualScreenWidth;
	virtualScreenHeight = requestedVirtualScreenHeight;

	float32 w, h;
	w = (float32)virtualScreenWidth / (float32)screenWidth;
	h = (float32)virtualScreenHeight / (float32)screenHeight;
	drawOffset.x = drawOffset.y = 0;
	float32 desD = 10000.0f;
	if(w > h)
	{
		physicalToVirtual = w;
		virtualToPhysical = (float32)screenWidth / (float32)virtualScreenWidth;
		if (fixedProportions)
		{
			drawOffset.y = 0.5f * ((float32)screenHeight - (float32)Core::Instance()->GetVirtualScreenHeight() * virtualToPhysical);
		}
		else
		{
			virtualScreenHeight = screenHeight * physicalToVirtual;
		}
		for (int i = 0; i < (int)allowedSizes.size(); i++) 
		{
			allowedSizes[i].toVirtual = (float32)virtualScreenWidth / (float32)allowedSizes[i].width;
			allowedSizes[i].toPhysical = (float32)screenWidth / (float32)allowedSizes[i].width;
			if (fabs(allowedSizes[i].toPhysical - 1.0f) < desD) 
			{
				desD = fabsf(allowedSizes[i].toPhysical - 1.0f);
				desirableIndex = i;
			}
		}
	}
	else
	{
		physicalToVirtual = h;
		virtualToPhysical = (float32)screenHeight / (float32)virtualScreenHeight;
		if (fixedProportions)
		{
			drawOffset.x = 0.5f * ((float32)screenWidth - (float32)Core::Instance()->GetVirtualScreenWidth() * virtualToPhysical);
		}
		else
		{
			virtualScreenWidth = screenWidth * physicalToVirtual;
		}
		for (int i = 0; i < (int)allowedSizes.size(); i++) 
		{
			allowedSizes[i].toVirtual = (float32)virtualScreenHeight / (float32)allowedSizes[i].height;
			allowedSizes[i].toPhysical = (float32)screenHeight / (float32)allowedSizes[i].height;
			if (fabs(allowedSizes[i].toPhysical - 1.0f) < desD) 
			{
				desD = fabsf(allowedSizes[i].toPhysical - 1.0f);
				desirableIndex = i;
			}
		}
	}
	
	drawOffset.y = floorf(drawOffset.y);
	drawOffset.x = floorf(drawOffset.x);

	UIControlSystem::Instance()->CalculateScaleMultipliers();

    if(enabledReloadResourceOnResize)
    {
        Sprite::ValidateForSize();
        TextBlock::ScreenResolutionChanged();
    }
			
//	Logger::FrameworkDebug("[Core] CalculateScaleMultipliers desirableIndex: %d", desirableIndex);
		
}
	
float32 Core::GetResourceToPhysicalFactor(int32 resourceIndex)
{
	DVASSERT(resourceIndex < (int32)allowedSizes.size());
	return allowedSizes[resourceIndex].toPhysical;
}
float32 Core::GetResourceToVirtualFactor(int32 resourceIndex)
{
	DVASSERT(resourceIndex < (int32)allowedSizes.size());
	return allowedSizes[resourceIndex].toVirtual;
}
const String& Core::GetResourceFolder(int32 resourceIndex)
{
	DVASSERT(resourceIndex < (int32)allowedSizes.size());
	return allowedSizes[resourceIndex].folderName;
}
int32 Core::GetDesirableResourceIndex()
{
	return desirableIndex;
}
int32 Core::GetBaseResourceIndex()
{
	return 0;
}
	
const Vector2 &Core::GetPhysicalDrawOffset()
{
	return drawOffset;
}

	
	

void Core::SetPhysicalScreenSize(int32 width, int32 height)
{
//	Logger::Info("Setting physical screen size to %dx%d", width, height);
	screenWidth = (float32)width;
	screenHeight = (float32)height;
	needTorecalculateMultipliers = true;
}
	
void Core::SetVirtualScreenSize(int32 width, int32 height)
{
	requestedVirtualScreenWidth = virtualScreenWidth = (float32)width;
	requestedVirtualScreenHeight = virtualScreenHeight = (float32)height;
	needTorecalculateMultipliers = true;
}

void Core::SetProportionsIsFixed( bool needFixed )
{
	fixedProportions = needFixed;
	needTorecalculateMultipliers = true;
}


void Core::RegisterAvailableResourceSize(int32 width, int32 height, const String &resourcesFolderName)
{
	Core::AvailableSize newSize;
	newSize.width = width;
	newSize.height = height;
	newSize.folderName = resourcesFolderName;
	
	allowedSizes.push_back(newSize);
}

void Core::UnregisterAllAvailableResourceSizes()
{
	allowedSizes.clear();
}
	
float32 Core::GetPhysicalScreenWidth()
{
	return screenWidth;
}
float32 Core::GetPhysicalScreenHeight()
{
	return screenHeight;
}
	
float32 Core::GetVirtualScreenWidth()
{
	return virtualScreenWidth;
}
float32 Core::GetVirtualScreenHeight()
{
	return virtualScreenHeight;
}
	
float32 Core::GetVirtualScreenXMin()
{
	return -(drawOffset.x * physicalToVirtual);
}
	
float32 Core::GetVirtualScreenXMax()
{
	return ((float32)(screenWidth - drawOffset.x) * physicalToVirtual);
}
	
float32 Core::GetVirtualScreenYMin()
{
	return -(drawOffset.y * physicalToVirtual);
}
	
float32 Core::GetVirtualScreenYMax()
{
	return ((float32)(screenHeight - drawOffset.y) * physicalToVirtual);
}
	
Core::eScreenMode Core::GetScreenMode()
{
	Logger::FrameworkDebug("[Core::GetScreenMode] return screen mode MODE_UNSUPPORTED");
	return MODE_UNSUPPORTED;
}

void Core::SwitchScreenToMode(eScreenMode screenMode)
{
	Logger::FrameworkDebug("[Core::SwitchScreenToMode] do not supported by platform implementation of core");
}

void Core::GetAvailableDisplayModes(List<DisplayMode> & availableModes)
{	

}
void Core::ToggleFullscreen()
{
	
}

DisplayMode Core::FindBestMode(const DisplayMode & requestedMode)
{
	List<DisplayMode> availableDisplayModes;
	GetAvailableDisplayModes(availableDisplayModes);

	DisplayMode bestMatchMode;

	bestMatchMode.refreshRate = -1;
	for (List<DisplayMode>::iterator it = availableDisplayModes.begin(); it != availableDisplayModes.end(); ++it)
	{
		DisplayMode & availableMode = *it;
		if ((availableMode.width == requestedMode.width) && (availableMode.height == requestedMode.height))
		{
			// if first mode found replace
			if (bestMatchMode.refreshRate == -1)
				bestMatchMode = availableMode;

			if (availableMode.bpp > bestMatchMode.bpp) // find best match with highest bits per pixel
			{
				bestMatchMode = availableMode;
			}
		}
	}

	if (bestMatchMode.refreshRate == -1) // haven't found any mode
	{
		int32 minDiffWidth = 0;
		int32 minDiffHeight = 0;
		float32 requestedAspect = (requestedMode.height>0?(float32)requestedMode.width/(float32)requestedMode.height:1.0f);
		float32 minDiffAspect = 0;

		for (List<DisplayMode>::iterator it = availableDisplayModes.begin(); it != availableDisplayModes.end(); ++it)
		{
			DisplayMode & availableMode = *it;

			int32 diffWidth = abs(availableMode.width - requestedMode.width);
			int32 diffHeight = abs(availableMode.height - requestedMode.height);

			float32 availableAspect = (availableMode.height>0?(float32)availableMode.width/(float32)availableMode.height:1.0f);
			float32 diffAspect = fabsf(availableAspect - requestedAspect);

//			if (diffWidth >= 0 && diffHeight >= 0)
			{
				// if first mode found replace
				if (bestMatchMode.refreshRate == -1)
				{
					minDiffWidth = diffWidth;
					minDiffHeight = diffHeight;
					minDiffAspect = diffAspect;
				}

				if(diffAspect<=(minDiffAspect+0.01f))
				{
					if((diffAspect+0.01f)<minDiffAspect)
					{
						// aspect changed, clear min diff
						minDiffWidth = diffWidth;
						minDiffHeight = diffHeight;
					}

					minDiffAspect = diffAspect;

					//int32 curDiffWidth = availableMode.width - bestMatchMode.width;
					//int32 curDiffHeight = availableMode.height - bestMatchMode.height;

					//if (diffWidth + diffHeight <= curDiffWidth + curDiffHeight)
					if (diffWidth + diffHeight <= minDiffWidth + minDiffHeight)
					{
						minDiffWidth = diffWidth;
						minDiffHeight = diffHeight;

						if (availableMode.bpp >= bestMatchMode.bpp) // find best match with highest bits per pixel
						{
							bestMatchMode = availableMode;
						}
					}
				}
			}
		}
	}

	if (bestMatchMode.refreshRate == -1) // haven't found any mode
	{
		int maxRes = 0;
		for (List<DisplayMode>::iterator it = availableDisplayModes.begin(); it != availableDisplayModes.end(); ++it)
		{
			DisplayMode & availableMode = *it;

			//int32 diffWidth = availableMode.width ;
			//int32 diffHeight = availableMode.height - requestedMode.height;
			if (availableMode.width + availableMode.height + availableMode.bpp > maxRes)
			{
				maxRes = availableMode.width + availableMode.height + availableMode.bpp;
				bestMatchMode = availableMode;
			}
		}
	}
	return bestMatchMode;
}

DisplayMode Core::GetCurrentDisplayMode()
{
	return DisplayMode();
}

void Core::Quit()
{
    exit(0);
	Logger::FrameworkDebug("[Core::Quit] do not supported by platform implementation of core");
}
	
void Core::SetApplicationCore(ApplicationCore * _core)
{
	core = _core;
}

ApplicationCore * Core::GetApplicationCore()
{
	return core;
}
	
	
void Core::SystemAppStarted()
{
	if (Core::Instance()->NeedToRecalculateMultipliers()) 
	{
		Core::Instance()->CalculateScaleMultipliers();
		/*  Question to Hottych: Does it really necessary here? 
            RenderManager::Instance()->SetRenderOrientation(Core::Instance()->GetScreenOrientation());
         */
	}

	if (core)core->OnAppStarted();
    
#ifdef __DAVAENGINE_AUTOTESTING__
    AutotestingSystem::Instance()->OnAppStarted();
#endif //__DAVAENGINE_AUTOTESTING__
}
	
void Core::SystemAppFinished()
{
#ifdef __DAVAENGINE_AUTOTESTING__
    AutotestingSystem::Instance()->OnAppFinished();
#endif //__DAVAENGINE_AUTOTESTING__    
	if (core)core->OnAppFinished();
}


void Core::SystemProcessFrame()
{
#ifdef __DAVAENGINE_NVIDIA_TEGRA_PROFILE__
	static bool isInit = false;
	static EGLuint64NV frequency;
	static PFNEGLGETSYSTEMTIMENVPROC eglGetSystemTimeNV;
	static PFNEGLGETSYSTEMTIMEFREQUENCYNVPROC eglGetSystemTimeFrequencyNV;
	if (!isInit)
	{
		eglGetSystemTimeNV = (PFNEGLGETSYSTEMTIMENVPROC) eglGetProcAddress("eglGetSystemTimeNV");
		eglGetSystemTimeFrequencyNV = (PFNEGLGETSYSTEMTIMEFREQUENCYNVPROC) eglGetProcAddress("eglGetSystemTimeFrequencyNV");
		if (!eglGetSystemTimeNV || !eglGetSystemTimeFrequencyNV)
		{
			DVASSERT(!"Error export eglGetSystemTimeNV, eglGetSystemTimeFrequencyNV");
			exit(0);
		}
		frequency = eglGetSystemTimeFrequencyNV();
	}
	EGLuint64NV start = eglGetSystemTimeNV() / frequency;
#endif //__DAVAENGINE_NVIDIA_TEGRA_PROFILE__
    Stats::Instance()->BeginFrame();
    TIME_PROFILE("Core::SystemProcessFrame");
    
	if (!core) return;
	if (!isActive)return;
	
	SystemTimer::Instance()->Start();

	/**
		Check if device not in lost state first / after that be
	*/
	if (!RenderManager::Instance()->IsDeviceLost())
	{
// #ifdef __DAVAENGINE_DIRECTX9__
// 		if(firstRun)
// 		{
// 			core->BeginFrame();
// 			firstRun = false;
// 		}
// #else
        InputSystem::Instance()->OnBeforeUpdate();
		core->BeginFrame();
//#endif

#if !defined(__DAVAENGINE_ANDROID__)
		RenderResource::SaveAllResourcesToSystemMem();
#endif //#if !defined(__DAVAENGINE_ANDROID__)

		// recalc frame inside begin / end frame
		if (Core::Instance()->needTorecalculateMultipliers) 
		{
			Core::Instance()->CalculateScaleMultipliers();
			RenderManager::Instance()->SetRenderOrientation(screenOrientation);
            UIScreenManager::Instance()->ScreenSizeChanged();
            UIControlSystem::Instance()->ScreenSizeChanged();
		}

		float32 frameDelta = SystemTimer::Instance()->FrameDelta();
        SystemTimer::Instance()->UpdateGlobalTime(frameDelta);

		if(Replay::IsRecord())
		{
			Replay::Instance()->RecordFrame(frameDelta);
		}
		if(Replay::IsPlayback())
		{
			UIControlSystem::Instance()->ReplayEvents();
			frameDelta = Replay::Instance()->PlayFrameTime();
			if(Replay::IsPlayback()) //can be unset in previous string
			{
				SystemTimer::Instance()->SetFrameDelta(frameDelta);
			}
		}
		
		JobManager::Instance()->Update();
		core->Update(frameDelta);
        InputSystem::Instance()->OnAfterUpdate();
		core->Draw();

		core->EndFrame();
// #ifdef __DAVAENGINE_DIRECTX9__
// 		core->BeginFrame();
// #endif
	}
    Stats::Instance()->EndFrame();
	globalFrameIndex++;
	
#ifdef __DAVAENGINE_NVIDIA_TEGRA_PROFILE__
	EGLuint64NV end = eglGetSystemTimeNV() / frequency;
	EGLuint64NV interval = end - start;
#endif //__DAVAENGINE_NVIDIA_TEGRA_PROFILE__
}

	
void Core::GoBackground(bool isLock)
{
#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__) 
	if (core)
    {
        if(isLock)
        {
            core->OnDeviceLocked();
        }
        else
        {
            core->OnBackground();
        }
    }
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
}

void Core::GoForeground()
{
#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
	if (core)
	{
		core->OnForeground();
	}
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
}

uint32 Core::GetGlobalFrameIndex()
{
	return globalFrameIndex;
}

float32 GetScreenWidth()
{
	return Core::Instance()->GetVirtualScreenWidth();
}

float32 GetScreenHeight()
{
	return Core::Instance()->GetVirtualScreenHeight();
}
	
void Core::SetCommandLine(int argc, char *argv[])
{
    commandLine.reserve(argc);
	for (int k = 0; k < argc; ++k)
		commandLine.push_back(argv[k]);
}

Vector<String> & Core::GetCommandLine()
{
	return commandLine;
}
	
bool Core::IsConsoleMode()
{
	return isConsoleMode;
}
	
void Core::EnableConsoleMode()
{
	isConsoleMode = true;
}

bool Core::NeedToRecalculateMultipliers()
{
	return needTorecalculateMultipliers;
}

void Core::SetIsActive(bool _isActive)
{
	isActive = _isActive;
}

#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_WIN32__)    
Core::eDeviceFamily Core::GetDeviceFamily()
{
    return DEVICE_DESKTOP;
}
#endif //#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_WIN32__)    
    
void Core::EnableReloadResourceOnResize(bool enable)
{
    enabledReloadResourceOnResize = enable;
}
    
uint32 Core::GetScreenDPI()
{
	return DPIHelper::GetScreenDPI();
}

void Core::SetIcon(int32 /*iconId*/)
{
};

DAVA::float32 Core::GetRequestedVirtualScreenWidth()
{
    return requestedVirtualScreenWidth;
}

DAVA::float32 Core::GetRequestedVirtualScreenHeight()
{
    return requestedVirtualScreenHeight;
}

};
