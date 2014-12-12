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

/**
	\defgroup core Core
	Application entry point and place where you can find all information about platform indepedent and platform dependent initialization and 
    platform functions you can use later during app execution.  
*/
namespace DAVA 
{
	

class CommandHistory
{
public:        
    struct Command
    {
        enum CommandType
        {
            CHC_UNKNOWN,
            CHC_FRAME_START,
            CHC_FRAME_END,
            CHC_JOB_MAIN_QUE,
            CHC_JOB_PERFORM,
            CHC_SPRITE_DRAW,
            CHC_RDO_BUILD_BUFFERS_INTERNAL,
            CHC_RDO_GL_GEN_BUFFER,
            CHC_RDO_GL_BUFFER_DATA,
            CHC_RDO_GL_BIND_BUFFER,
            CHC_RDO_GL_DELETE_BUFFER,
            CHC_RM_MAKE_SCREENSHOT,
            CHC_RM_SET_VIEWPORT,
            CHC_RM_CULL_ORDER,
            CHC_RM_DRAW_ARRAYS,
            CHC_RM_DRAW_ELEMENTS,
            CHC_RM_CLEAR_COLOR,
            CHC_RM_CLEAR_DEPTH,
            CHC_RM_CLEAR_STENCIL,
            CHC_RM_CLEAR_ALL,
            CHC_RM_SET_HW_CLIP,
            CHC_RM_SET_HW_RT_SPRITE,
            CHC_RM_SET_HW_RT_TEXTURE,
            CHC_RM_DISCARD_FB,
            CHC_RM_HW_BIND_BUFFER,
            CHC_RM_HW_DELETE_BUFFER,
            CHC_RM_GL_GEN_BUFFER,
            CHC_RM_GL_BUFFER_DATA,
            CHC_RM_GL_BIND_BUFFER,
            CHC_RM_GL_DELETE_BUFFER,

            CHC_RM_HW_ATTACH_RENDERDATA,
            CHC_RM_GL_ENABLE_ATTRIB_POINTER,
            CHC_RM_GL_DISABLE_ATTRIB_POINTER,
            CHC_RM_GL_SET_ATTRIB_POINTER,

            CHC_RM_HW_BIND_TEXTURE,
            CHC_RM_HW_BIND_FBO,
            CHC_RM_HW_DISCARD_DEPTH,

            CHC_RS_FLUSH, 
            CHC_RS_FLUSH_STATE,
            CHC_RS_FLUSH_TS,
            CHC_RS_FLUSH_SHADER,
            CHC_RS_HW_SET_COLOR_MASK,
            CHC_RS_HW_SET_STENCIL,
            CHC_RS_HW_SET_BLEND,
            CHC_RS_HW_SET_CULL,
            CHC_RS_HW_SET_CULL_MODE,
            CHC_RS_HW_SET_BLEND_MODE,
            CHC_RS_HW_SET_TEXTURE_LEVEL,
            CHC_RS_HW_SET_DEPTH_TEST,
            CHC_RS_HW_SET_DEPTH_WRITE,
            CHC_RS_HW_SET_DEPTH_FUNC,
            CHC_RS_HW_SET_SCISSOR_TEST,
            CHC_RS_HW_SET_FILL_MODE,
            CHC_RS_HW_SET_STENCIL_FUNC,
            CHC_RS_HW_SET_STENCIL_OP,
            CHC_SHADER_RECOMPILE,
            CHC_SHADER_LINK,
            CHC_SHADER_SET_UNIFORM_BY_INDEX,
            CHC_SHADER_SET_UNIFORM_BY_UNIFORM,
            CHC_SHADER_DELETE,
            CHC_SHADER_COMPILE,
            CHC_SHADER_BIND,
            CHC_SHADER_UNBIND,
            CHC_SHADER_BIND_DP_START,
            CHC_SHADER_BIND_DP,
            CHC_TEXTURE_RELEASE,
            CHC_TEXTURE_CREATE_FROM_DATA,
            CHC_TEXTURE_CREATE_TEXT_FROM_DATA,
            CHC_TEXTURE_TEX_IMAGE,
            CHC_TEXTURE_SET_WRAP,
            CHC_TEXTURE_SET_FILTER,
            CHC_TEXTURE_GEN_MM,
            CHC_TEXTURE_GEN_PIX,
            CHC_TEXTURE_FLUSH2RENDERER,
            CHC_TEXTURE_CREATE_FBO,
            CHC_TEXTURE_CREATE_FBO_BUFFERS,
            CHC_TEXTURE_READ_DATA_TO_IMG,
            CHC_TEXTURE_GEN_ID,

            CHC_SCENE_DRAW,
            CHC_UICONTROLSYSTEM_DRAW,



            CHC_COMMANDS_END
        };

        CommandType command;
        int32 param1, param2, param3;
        const void *pointer;
        Command():command(CHC_UNKNOWN), param1(0), param2(0), param3(0), pointer(NULL){}
        Command(CommandType ct, int32 p1, int32 p2, int32 p3, const void* pp):command(ct), param1(p1), param2(p2), param3(p3), pointer(pp){}
    };

    CommandHistory();
    void AddCommand(Command::CommandType ct, int32 p1=0, int32 p2=0, int32 p3=0, const void* pp=NULL);
    Vector<Command> commandBuffer;
    int32 currSlot;
};
	
#if defined(__DAVAENGINE_WIN32__)
	typedef HINSTANCE AppHandle;
#elif defined(__DAVAENGINE_ANDROID__)
    typedef struct android_app* AppHandle;
#else
	typedef uint32 AppHandle;
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
	
	struct AvailableSize
	{
		AvailableSize()
			:	width(0)
			,	height(0)
			,	toVirtual(0)
			,	toPhysical(0)
		{

		}
		int32 width;
		int32 height;
		String folderName;
		float32 toVirtual;
		float32 toPhysical;
	};
	
	enum eScreenOrientation
	{
			SCREEN_ORIENTATION_TEXTURE = -1// uses only for the draw to texture purposes
		,	SCREEN_ORIENTATION_LANDSCAPE_RIGHT = 0
		,	SCREEN_ORIENTATION_LANDSCAPE_LEFT
		,	SCREEN_ORIENTATION_PORTRAIT
		,	SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN
        ,   SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE
        ,   SCREEN_ORIENTATION_PORTRAIT_AUTOROTATE
	};
    
    enum eRenderer
    {
        RENDERER_OPENGL_ES_1_0, // 1.0 compatible OpenGL ES. Old generation iOS / Android devices. 
        RENDERER_OPENGL_ES_2_0, // 2.0 compatible OpenGL ES. New generation iOS / Android devices. 
        RENDERER_OPENGL_ES_3_0, // 3.0 compatible OpenGL ES. New generation iOS / Android devices.
        RENDERER_OPENGL,        // here we assuming that it's 2.0 compatible. Renderer for MacOS X.
        RENDERER_DIRECTX9,      // only renderer that works on win platforms right now. 
//        RENDERER_DIRECTX10,   // written for self-motivation
//        RENDERER_DIRECTX11,   // written for self-motivation
    };
    
	
	Core();
	virtual ~Core();
	
	enum eScreenMode
	{
		MODE_UNSUPPORTED = 0,	// for all devices that do not support 
		MODE_FULLSCREEN, 
		MODE_WINDOWED,
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
    void CreateRenderManager();
    // Should be called after full release
	void ReleaseSingletons();

	Vector<String> & GetCommandLine(); 
	bool IsConsoleMode();

    CommandHistory commandHistory;
	
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
	virtual void SwitchScreenToMode(eScreenMode screenMode); 
	
	/**
		\brief Get list of available display modes supported by hardware
		\param[out] availableModes list of available modes that is supported by hw
	*/
	virtual void GetAvailableDisplayModes(List<DisplayMode> & availableModes);
	
	/**
		
	*/
	virtual void ToggleFullscreen();

	/**
		\brief Find mode that matches best to the mode you've requested
		\param[in] requestedMode mode you want to get
		\returns best mode found in current HW
	*/
	virtual DisplayMode FindBestMode(const DisplayMode & requestedMode);

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
	
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	static bool IsAutodetectContentScaleFactor();
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
//	static void SetContentScaleFactor(float scaleFactor);//sets content scale factor
//	static float GetContentScaleFactor();//returns content scale factor
//	static float GetInverseContentScaleFactor();//returns one divided by content scale factor(0.5 for scale factor 2.0)
	
	static float32 GetVirtualToPhysicalFactor();
	static float32 GetPhysicalToVirtualFactor();
	
	
	virtual Core::eScreenOrientation GetScreenOrientation();
	virtual void CalculateScaleMultipliers();
	
	virtual void SetPhysicalScreenSize(int32 width, int32 height);//!< May be used only by the system
	virtual void SetVirtualScreenSize(int32 width, int32 height);//!< Sets virtual screen size. You need to set size what takes into account screen orientation modifier
	virtual void SetProportionsIsFixed(bool needFixed);
	virtual void RegisterAvailableResourceSize(int32 width, int32 height, const String &resourcesFolderName);//!< Registers available sizes of resources. Can be called many times.
	virtual void UnregisterAllAvailableResourceSizes();
	

	virtual float32 GetPhysicalScreenWidth();//returns physical size what don't take intpo account screen orientation
	virtual float32 GetPhysicalScreenHeight();//returns physical size what don't take intpo account screen orientation
	virtual const Vector2 &GetPhysicalDrawOffset();

	virtual float32 GetVirtualScreenWidth();
	virtual float32 GetVirtualScreenHeight();
    virtual float32 GetRequestedVirtualScreenWidth();
    virtual float32 GetRequestedVirtualScreenHeight();
	virtual float32 GetVirtualScreenXMin();
	virtual float32 GetVirtualScreenXMax();
	virtual float32 GetVirtualScreenYMin();
	virtual float32 GetVirtualScreenYMax();
	
	virtual float32 GetResourceToPhysicalFactor(int32 resourceIndex);
	virtual float32 GetResourceToVirtualFactor(int32 resourceIndex);
	virtual const String& GetResourceFolder(int32 resourceIndex);
	virtual int32 GetDesirableResourceIndex();
	virtual int32 GetBaseResourceIndex();
	
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

    inline bool IsAutotesting() {return isAutotesting;}

    inline bool IsActive();
	void SetIsActive(bool isActive);
	
	virtual void GoBackground(bool isLock);
	virtual void GoForeground();
	
	/**
		\brief Checks if framework needs to recalculate scale multipliers.
	*/
	bool NeedToRecalculateMultipliers();
    
	/**
     \brief Get device familty
     */
    eDeviceFamily GetDeviceFamily();
    
    void EnableReloadResourceOnResize(bool enable);
	
	// Needs to be overriden for the platforms where it has sence (MacOS only for now).
	virtual void* GetOpenGLView() { return NULL; };
	
	void EnableConsoleMode();

protected:
	int32 screenOrientation;

private:
	float32 screenWidth;
	float32 screenHeight;	
	
	int desirableIndex;
	
	float32 virtualScreenWidth;
	float32 virtualScreenHeight;
	float32 requestedVirtualScreenWidth;
	float32 requestedVirtualScreenHeight;
	bool fixedProportions;
	
	Vector<AvailableSize> allowedSizes;
	bool needTorecalculateMultipliers;
	
	static float32 virtualToPhysical;
	static float32 physicalToVirtual;
	static Vector2 drawOffset;
	
    KeyedArchive * options;

	bool isActive;
	bool isAutotesting;

	uint32 globalFrameIndex;

	bool firstRun;//call begin frame 1st time
	
	void SetCommandLine(int argc, char *argv[]);
	Vector<String> commandLine;
	bool isConsoleMode;
    
    void CheckDataTypeSizes();
    template <class T> void CheckType(T t, int32 expectedSize, const char * typeString);
    
    
    bool enabledReloadResourceOnResize;
};

float32 GetScreenWidth();
float32 GetScreenHeight();
    
inline bool Core::IsActive()
{
    return isActive;
}

};



#endif // __DAVAENGINE_CORE_H__
