#ifndef __DAVAENGINE_APPLICATION_CORE__
#define __DAVAENGINE_APPLICATION_CORE__

#if !defined(__DAVAENGINE_COREV2__)

#include "Base/BaseObject.h"
#include "Render/RHI/rhi_Type.h"

namespace DAVA
{
/**
	\ingroup core
	\brief Class that defines high-level application logic. 
	This is main class of every application or game that made on our SDK. 
	
	To create your own application you need to perform the following steps:
 
	1. Define FrameworkDidLaunched and FrameworkWillTerminate functions
	\code
	void FrameworkDidLaunched()
	{
		//	Create keyed archive for application options
		KeyedArchive * appOptions = new KeyedArchive();
	#if defined(__DAVAENGINE_IPHONE__)
		// set application base orientation (should be the same as in Info.plist)
		appOptions->SetInt("orientation", Core::SCREEN_ORIENTATION_PORTRAIT);

		// set virtual resolution you want to work in
		DAVA::Core::Instance()->SetVirtualScreenSize(320, 480);
		
		// register resources and their resolutions
		DAVA::Core::Instance()->RegisterAvailableResourceSize(320, 480, "Gfx");
	#else
		// set client area size for the windowed application
		appOptions->SetInt("width",	920);
		appOptions->SetInt("height", 690);

		appOptions->SetInt("fullscreen.width",	1024);
		appOptions->SetInt("fullscreen.height", 768);

		// this flag means that we start from windowed mode
		appOptions->SetInt("fullscreen", 0);
		
		// 32 bits per pixel
		appOptions->SetInt("bpp", 32); 

		DAVA::Core::Instance()->SetVirtualScreenSize(920, 690);
		DAVA::Core::Instance()->RegisterAvailableResourceSize(920, 690, "Gfx");
	#endif 
		
		// create our class that inherited from ApplicationCore
		GameCore * core = new GameCore();
		// set it as application core
		DAVA::Core::SetApplicationCore(core);
		// set launch options
		DAVA::Core::SetOptions(appOptions);
	}

	void FrameworkWillTerminate()
	{


	}
	\endcode
	
	2. Implement all virtual functions of ApplicationCore
	\code
	void GameCore::OnAppStarted()
	{
		Renderer::SetDesiredFPS(30);

		mainMenuScreen.Set(new MainMenuScreen());
		gameScreen.Set(new GameScreen());
		winScreen = newref(WinScreen);

		UIScreenManager::Instance()->RegisterScreen(SCREEN_MAIN_MENU, mainMenuScreen.Get());
		UIScreenManager::Instance()->RegisterScreen(SCREEN_GAME, gameScreen.Get());
		UIScreenManager::Instance()->RegisterScreen(SCREEN_WIN, winScreen.Get());

		UIScreenManager::Instance()->SetFirst(SCREEN_GAME);
	}

	void GameCore::OnAppFinished()
	{
		// Release RefPtr pointers to avoid memory leaks
		mainMenuScreen = 0;
		gameScreen = 0;
	}

	void GameCore::OnSuspend()
	{
	}

	void GameCore::OnResume()
	{
	}

	void GameCore::OnBackground()
	{
	}

	void GameCore::BeginFrame()
	{
		ApplicationCore::BeginFrame();
		RenderManager::Instance()->ClearWithColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	void GameCore::Update(float32 timeElapsed)
	{	
		ApplicationCore::Update(timeElapsed);
	}

	void GameCore::Draw()
	{
		ApplicationCore::Draw();
	}
	\endcode
 */

class Thread;

class ApplicationCore : public BaseObject
{
protected:
    virtual ~ApplicationCore();

public:
    ApplicationCore();

    /**
		\brief Called when application is suspended or minimized.
		Stops main loop.
	 */
    virtual void OnSuspend();

    /**
		\brief Called when application is resumed after suspend or minimization.
		Resumes main loop.
	 */
    virtual void OnResume();

    /**
     \brief Called after entering fullscreen.
     */
    virtual void OnEnterFullscreen();

    /**
     \brief Called after exiting fullscreen.
     */
    virtual void OnExitFullscreen();

    /**
		\brief Called time to time from separate thread (not main) when application is Suspended.
	 */
    virtual void OnBackgroundTick();

    /**
		\brief Called when application is going to quit.
		Called after quit event has come from operating system. If false is returned, application will quit in normal way (all destructors are called).
		Is true is returned, application will fast quit (no desructors are called). Fast quit is usually used to prevent crash on quit while loading transition is in progress.
		Return false in default implementation.

		\returns true for fast quit, false for normal quit
	 */
    virtual bool OnQuit();

protected:
    /**
		\brief Called immediately after application initialized and all singletons are created. 
		This function is second initialization function of your application. First initialization function is FrameworkDidLaunched and it actually allow you to set 
		some important things like your application resolution, set available graphics resources folders and do preliminary initialization.
		This function is more related to your game or application logic. 
		You should overload this function in your GameCore and in overloaded function you should create all application screens or at least one of the screens.
		After creation you should register all of them, and then set first launch screen.
		
		Example: 
		\code
			void GameCore::OnAppStarted()
			{
				Renderer::SetDesiredFPS(30);

				mainMenuScreen.Set(new MainMenuScreen());
				gameScreen.Set(new GameScreen());
				winScreen = newref(WinScreen);

				UIScreenManager::Instance()->RegisterScreen(SCREEN_MAIN_MENU, mainMenuScreen.Get());
				UIScreenManager::Instance()->RegisterScreen(SCREEN_GAME, gameScreen.Get());
				UIScreenManager::Instance()->RegisterScreen(SCREEN_WIN, winScreen.Get());

				UIScreenManager::Instance()->SetFirst(SCREEN_GAME);
			}
		\endcode
	 */
    virtual void OnAppStarted() = 0;
    /**
		\brief Called when user requested to quit from application. 
		You should put all deinitialization in this function. Here you can release all objects. 
		Framework can help you to find memory leaks but to use memory leak detection you should release all objects carefully. 
		
		We do not recommend to save game progress in this function, because on some platforms it can create problems. 
		Our recommendation to perform in-game progress saves during the game immediately after changes that are important. 
	 */
    virtual void OnAppFinished();

    /**
		\brief Called when application goes to background on mobile platforms
	 */
    virtual void OnBackground();

    /**
	 \brief Called when application returns to foreground on mobile platforms
	 */
    virtual void OnForeground();

    /**
     \brief Called when application goes to background due to device lock on iOS platforms
	 */
    virtual void OnDeviceLocked();

    /**
     \brief Called when application lost focus (desktop platforms)
     */
    virtual void OnFocusLost();

    /**
     \brief Called when application has received focus (desktop platforms)
     */
    virtual void OnFocusReceived();

    /**	
		\brief this function is called every frame to let you update your application. 
		Normally this function can handle high-level tasks that is common between all application screens. 
		Logic of the particular game screen should be inside that screen and it's Update function. 
	 
		To modify the frequency of Update calls you can use \ref Renderer::SetDesiredFPS() function
	 
		\param[in] timeElapsed time in seconds that passed from the previous frame
	 */
    virtual void Update(float32 timeElapsed);
    /**
		\brief Called when application is ready to draw the frame
		In this function you can perform draw. Normally you should draw inside your particular screen but in some cases you can utilize this function when you need to draw something that will work on every screen.
	 */
    virtual void Draw();
    /**
		\brief Called before draw to let you prepare to the rendering
	 */
    virtual void BeginFrame();
    /**
		\brief Called after draw is finished.
	 */
    virtual void EndFrame();

    /**
        \brief Callback to handle situation where rendering is not possible anymore.
        Could be called from any thread which uses render functions (could be main, render, loading thread, etc)
        Application should be gracefully closed with all required actions (dump memory, update analytics, etc),
        Assumed to be "no-return" callback. Application behaviour after this callback assumed to be undefined.
    */
    virtual void OnRenderingIsNotPossible(rhi::RenderingError);

#if defined(__DAVAENGINE_ANDROID__)
protected:
    /**
		\brief Should be started only when Main thread is stopped
	 */
    void StartBackgroundTicker(uint32 tickPeriod = 250);
    /**
		\brief Should be stopped before Main thread start
	 */
    void StopBackgroundTicker();

private:
    void BackgroundTickerHandler(BaseObject* caller, void* callerData, void* userData);
#endif

private:
    friend class Core;

#if defined(__DAVAENGINE_ANDROID__)
private:
    Thread* backgroundTicker;
    volatile bool backgroundTickerFinishing;
    uint32 backgroundTickTimeMs;
#endif
};
};

#endif // !__DAVAENGINE_COREV2__
#endif
