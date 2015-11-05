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


#ifndef __DAVAENGINE_CORE_H__
#define __DAVAENGINE_CORE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/Singleton.h"
#include "Core/ApplicationCore.h"
#include "Core/DisplayMode.h"
#include "FileSystem/KeyedArchive.h"
#include "Render/RHI/rhi_Public.h"

/**
	\defgroup core Core
	Application entry point and place where you can find all information about platform indepedent and platform dependent initialization and 
    platform functions you can use later during app execution.  
*/
namespace DAVA 
{
#if defined(__DAVAENGINE_WIN32__)
    using AppHandle = HINSTANCE;
#elif defined(__DAVAENGINE_ANDROID__)
    using AppHandle = struct android_app*;
#else
    using AppHandle = uint32;
#endif 

/**
	\ingroup core
	\brief	Core is a main singleton that initialize everything under all of platforms. 
            It's a place where you can get some specific information about your application on every supported platform.
			To read about the process of application initialization check documentation for \ref ApplicationCore class. 
			
 
			Supported engine configuration options: 
				
			\section w32_macos Win32 / MacOS X 
			width: 1024<br/>
			height: 768<br/>
			fullscreen: 1<br/>
			bitsperpixel: 32<br/>
			 
			\section ios iOS
			orientation:	SCREEN_ORIENTATION_LANDSCAPE_RIGHT,
							SCREEN_ORIENTATION_LANDSCAPE_LEFT,
							SCREEN_ORIENTATION_PORTRAIT,
							SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN<br/>
 
            renderer:         
                RENDERER_OPENGL_ES_1_0, 
                RENDERER_OPENGL_ES_2_0, 
                RENDERER_OPENGL_ES_3_0,
                RENDERER_OPENGL,
                RENDERER_DIRECTX9       <br/>
           
			
			\section all All platforms
			zbuffer: 1	<br/>

			Specific implementation notes (for people who involved to development of platform dependant templates)
			Core::CreateSingletons must be always called from main thread of application or from main rendering thread.
			It's required to perform thread system initialization correctly. 
 */
class Core : public Singleton<Core>
{
public:

    enum eScreenOrientation
    {
        SCREEN_ORIENTATION_TEXTURE = -1,     // Use only for texture purpose drawings
        SCREEN_ORIENTATION_LANDSCAPE_RIGHT = 0,
        SCREEN_ORIENTATION_LANDSCAPE_LEFT,
        SCREEN_ORIENTATION_PORTRAIT,
        SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN,
        SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE,
        SCREEN_ORIENTATION_PORTRAIT_AUTOROTATE
    };

    Core();
    virtual ~Core();

    enum class eScreenMode
    {
        FULLSCREEN = 0, //<! True full screen
        WINDOWED_FULLSCREEN, //<! Windowed mode without border and full screen sized
        WINDOWED, //<! Windowed mode
    };

    enum eDeviceFamily
    {
        DEVICE_UNKNOWN = -1,
        DEVICE_HANDSET = 0,
        DEVICE_PAD, 
        DEVICE_DESKTOP
    };
    
    static int Run(int argc, char *argv[], AppHandle handle = 0);
    static int RunCmdTool(int argc, char *argv[], AppHandle handle = 0);

    // Should be called in platform initialization before FrameworkDidLaunched
    void CreateSingletons();
    // Should be called after framework did launched to initialize proper render manager
    void CreateRenderer();
    // Should be called after full release
    void ReleaseRenderer();
    void ReleaseSingletons();

    const Vector<String> & GetCommandLine(); 
    bool IsConsoleMode();

public:
    void SetOptions(KeyedArchive * archiveOfOptions);
    KeyedArchive * GetOptions();

	
	static void SetApplicationCore(ApplicationCore * core);
	static ApplicationCore * GetApplicationCore();

	
	// platform dependent functions that should be implemented
	virtual eScreenMode GetScreenMode();	// 
	
	/**
		\brief This function should perform switching from one mode to another (fullscreen => windowed and back)
		\param[in] screenMode mode of the screen we want to switch to
	*/
    virtual bool SetScreenMode(eScreenMode screenMode);

    /**
		\brief Get list of available display modes supported by hardware
		\param[out] availableModes list of available modes that is supported by hw
	*/
    virtual void GetAvailableDisplayModes(List<DisplayMode>& availableModes);

    /**
		\brief Find mode that matches best to the mode you've requested
		\param[in] requestedMode mode you want to get
		\returns best mode found in current HW
	*/
    virtual DisplayMode FindBestMode(const DisplayMode& requestedMode);

    /**
		\brief Get current display mode. This function return resolution of the current display mode enabled on the first (main) monitor
	*/
	virtual DisplayMode GetCurrentDisplayMode();

	/**
		\brief Quit from application & release all subsystems
	*/
	virtual void Quit();

    /**
		\brief Set icon for application's window.
		Windows: First of all, you should create icon resource through Project->Add Resource->Icon.
		param[in] iconId resource id for icon from resource.h file. For example, 101 for #define IDI_ICON1 101
	 */
    virtual void SetIcon(int32 iconId);

    virtual float32 GetScreenScaleMultiplier() const;
    virtual void SetScreenScaleMultiplier(float32 multiplier);

    virtual float32 GetScreenScaleFactor() const;

    virtual Core::eScreenOrientation GetScreenOrientation();

    virtual uint32 GetScreenDPI();
	
	/*
		\brief Mouse cursor for the platforms where it make sense (Win32, MacOS X) 
	 */

	
	/* This code disabled for now and left for the future
	MacOS X Version: it works right (commented in MainWindowController.mm) but it require convertaton to virtual coordinates
	For Win32 function not implemented yet, and I do not have time to implement it right now, so left that for the future.
     
     */
	/*
		\brief Function that return number of frame from the launch of the application
		
		This function supposed for such situations when you do not want to recompute something during one frame more than 
		once. So you can store frameIndex in your object and check have you updated it already or not. 
		By default this counter starts from frame with index 1, so you can initialize your variables by 0 if you want to 
		use this index. 
		
		Usage example: 
		\code
		uint32 updateFrameIndex = 0;
	 
		void UpdateFunction()
		{
			if (updateFrameIndex == )return; // no update
			updateCounter = Core::Instance()->GetGlobalFrameIndex();
	 
		}
	 
		\endcode
		
		\returns global frame index from the launch of your application
	 */
	uint32 GetGlobalFrameIndex();
	
	/*
		This function performs message on main thread 
		\param[in] message message to be performed
	 */
	//void PerformMessageOnMainThread(const Message & message, bool waitUntilDone = true);
	
	/*
		* FOR INTERNAL FRAMEWORK USAGE ONLY * 
		MUST BE CALLED FROM templates on different OS
	 */
	
	void SystemAppStarted();
	void SystemProcessFrame();
	void SystemAppFinished();

    inline bool IsActive();
	void SetIsActive(bool isActive);
	
	virtual void GoBackground(bool isLock);
	virtual void GoForeground();
    virtual void FocusLost();
    virtual void FocusReceived();

    /**
     \brief Get device familty
     */
    eDeviceFamily GetDeviceFamily();
	
	// Needs to be overriden for the platforms where it has sence (MacOS only for now).
    void* GetNativeView() const;
    void SetNativeView(void* nativeView);

    void EnableConsoleMode();

    rhi::InitParam rendererParams;

protected:
	int32 screenOrientation;

	void SetCommandLine(int argc, char *argv[]);
	void SetCommandLine(Vector<String>&& args);
    void SetCommandLine(const DAVA::String& cmdLine);

private:
    KeyedArchive * options;

	bool isActive;

	uint32 globalFrameIndex;

	bool firstRun;//call begin frame 1st time
	
	Vector<String> commandLine;
	bool isConsoleMode;
    void* nativeView;
};
    
inline bool Core::IsActive()
{
    return isActive;
}
    
};

#endif // __DAVAENGINE_CORE_H__
