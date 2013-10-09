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


#include "Autotesting/AutotestingSystemLua.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "AutotestingSystem.h"

#include "Utils/Utils.h"

//TODO: move all wrappers to separate class?
#include "Action.h"
#include "TouchAction.h"

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};

// directly include wrapped module here to compile only if __DAVAENGINE_AUTOTESTING__ is defined
//#include "AutotestingSystem_wrap.cxx"

//#include "VectorSwig.cpp"
//#include "UIControlSwig.cpp"
//#include "AutotestingSystemSwig.cpp"

extern "C" int luaopen_AutotestingSystem(lua_State *l);
extern "C" int luaopen_UIControl(lua_State *l); 
extern "C" int luaopen_Rect(lua_State *l);
extern "C" int luaopen_Vector(lua_State *l);
extern "C" int luaopen_KeyedArchive(lua_State *l);

namespace DAVA
{
        
AutotestingSystemLua::AutotestingSystemLua() : luaState(NULL), delegate(NULL), autotestingLocalizationSystem(NULL)
{
    autotestingLocalizationSystem = new LocalizationSystem();
}

AutotestingSystemLua::~AutotestingSystemLua()
{
    SafeRelease(autotestingLocalizationSystem);
    
    if(luaState)
    {
        lua_close(luaState);
        luaState = NULL;
    }
}

void AutotestingSystemLua::SetDelegate(AutotestingSystemLuaDelegate *_delegate)
{
    delegate = _delegate;
}
    
void AutotestingSystemLua::InitFromFile(const String &luaFilePath)
{
    Logger::FrameworkDebug("AutotestingSystemLua::InitFromFile luaFilePath=%s", luaFilePath.c_str());
    if(!luaState)
    {
        autotestingLocalizationSystem->SetCurrentLocale(LocalizationSystem::Instance()->GetCurrentLocale());
        autotestingLocalizationSystem->InitWithDirectory("~res:/Autotesting/Strings/");
        
        bool isOk = true;
        luaState = lua_open();
        luaL_openlibs(luaState);
        String errors;
        
        if(isOk)
        {
            isOk = LoadWrappedLuaObjects();
        }
        else
        {
            errors += " luaL_openlibs failed";
        }
        
        if(isOk)
        {
            isOk = RunScriptFromFile("~res:/Autotesting/Scripts/autotesting_api.lua");
        }
        else
        {
            errors += ", LoadWrappedLuaObjects failed";
        }
        
        FilePath pathToAutotesting = "~res:/Autotesting/";
        String setPackagePathScript = Format("SetPackagePath(\"%s\")", pathToAutotesting.GetAbsolutePathname().c_str());
        if(isOk)
        {
            isOk = RunScript(setPackagePathScript);
        }
        else
        {
            errors += ", autotesting_api.lua failed";
        }
        
        if(isOk)
        {
            isOk = LoadScriptFromFile(luaFilePath);
        }
        else
        {
            errors += ", " + setPackagePathScript + " failed";
        }
        
        if(isOk)
        {
            AutotestingSystem::Instance()->OnInit();
        }
        else
        {
            errors += ", " + luaFilePath + " failed";
        }
        
        if(isOk)
        {
            AutotestingSystem::Instance()->RunTests();
        }
        else
        {
            AutotestingSystem::Instance()->OnError(errors);
        }
    }
}

void AutotestingSystemLua::StartTest()
{
    Logger::FrameworkDebug("AutotestingSystemLua::StartTest");
    RunScript();
}
    
void AutotestingSystemLua::WaitForMaster()
{
    Logger::FrameworkDebug("AutotestingSystemLua::WaitForMaster");
    AutotestingSystem::Instance()->InitMultiplayer(false);
    AutotestingSystem::Instance()->RegisterHelperInDB();
}
    
void AutotestingSystemLua::WaitForHelpers(DAVA::int32 helpersCount)
{
    Logger::FrameworkDebug("AutotestingSystemLua::WaitForHelpers %d", helpersCount);
    AutotestingSystem::Instance()->InitMultiplayer(true);
    AutotestingSystem::Instance()->RegisterMasterInDB(helpersCount);
}

// Multiplayer API
void AutotestingSystemLua::WriteState(const String & device, const String & state)
{
	Logger::FrameworkDebug("AutotestingSystemLua::WriteState device=%s state=%s", device.c_str(), state.c_str());
	AutotestingSystem::Instance()->WriteState(device,state);
}

void AutotestingSystemLua::WriteCommand(const String & device, const String & state)
{
	Logger::FrameworkDebug("AutotestingSystemLua::WriteCommand device=%s command=%s", device.c_str(), state.c_str());
	AutotestingSystem::Instance()->WriteCommand(device,state);
}

String AutotestingSystemLua::ReadState(const String & device)
{
	Logger::FrameworkDebug("AutotestingSystemLua::ReadState device=%s", device.c_str());
	return AutotestingSystem::Instance()->ReadState(device);
}

String AutotestingSystemLua::ReadCommand(const String & device)
{
	Logger::FrameworkDebug("AutotestingSystemLua::ReadCommand device=%s", device.c_str());
	return AutotestingSystem::Instance()->ReadCommand(device);
}

void AutotestingSystemLua::InitializeDevice(const String & device)
{
	Logger::FrameworkDebug("AutotestingSystemLua::InitializeDevice device=%s", device.c_str());
	AutotestingSystem::Instance()->InitializeDevice(device);
}


void AutotestingSystemLua::Update(float32 timeElapsed)
{
    RunScript("ResumeTest()"); //TODO: time 
}
    
float32 AutotestingSystemLua::GetTimeElapsed()
{
    return SystemTimer::FrameDelta();
}
    
void AutotestingSystemLua::OnError(const String &errorMessage)
{
    Logger::FrameworkDebug("AutotestingSystemLua::OnError %s", errorMessage.c_str());
    AutotestingSystem::Instance()->OnError(errorMessage);
}
    
void AutotestingSystemLua::OnTestStep(const String &stepName, bool isPassed, const String &error)
{
    Logger::FrameworkDebug("AutotestingSystemLua::OnTestStep %s %d %s", stepName.c_str(), isPassed, error.c_str());
    AutotestingSystem::Instance()->OnTestStep(stepName, isPassed, error);
}

void AutotestingSystemLua::OnTestStart(const String &testName)
{
	Logger::FrameworkDebug("AutotestingSystemLua::OnTestStart %s", testName.c_str());
	AutotestingSystem::Instance()->OnTestStart(testName);
}

void AutotestingSystemLua::OnTestFinished()
{
    Logger::FrameworkDebug("AutotestingSystemLua::OnTestFinished");
    AutotestingSystem::Instance()->OnTestsFinished();
}
    
void AutotestingSystemLua::OnStepStart(const String &stepName)
{
	Logger::FrameworkDebug("AutotestingSystemLua::OnStepStart %s", stepName.c_str());
	AutotestingSystem::Instance()->OnStepStart(stepName);
}

void AutotestingSystemLua::Log(const String &level, const String &message)
{
	Logger::FrameworkDebug("AutotestingSystemLua::Log [%s]%s", level.c_str(), message.c_str());
	AutotestingSystem::Instance()->Log(level, message);
}

void AutotestingSystemLua::WriteString(const String & name, const String & text)
{
	Logger::FrameworkDebug("AutotestingSystemLua::WriteString name=%s text=%s", name.c_str(), text.c_str());
	AutotestingSystem::Instance()->WriteString(name, text);
}

String AutotestingSystemLua::ReadString(const String & name)
{
	Logger::FrameworkDebug("AutotestingSystemLua::ReadString name=%s", name.c_str());
	return AutotestingSystem::Instance()->ReadString(name);
}

bool AutotestingSystemLua::SaveKeyedArchiveToDB(const String &archiveName, KeyedArchive *archive, const String &docName)
{
	Logger::FrameworkDebug("AutotestingSystemLua::SaveKeyedArchiveToDB");
	return AutotestingSystem::Instance()->SaveKeyedArchiveToDB(archiveName, archive, docName);
}

String AutotestingSystemLua::MakeScreenshot()
{
	Logger::FrameworkDebug("AutotestingSystemLua::MakeScreenshot");
	AutotestingSystem::Instance()->MakeScreenShot();
	return AutotestingSystem::Instance()->GetScreenShotName();
}

UIControl *AutotestingSystemLua::GetScreen()
{
    return UIControlSystem::Instance()->GetScreen();
}

UIControl *AutotestingSystemLua::FindControl(const String &path)
{
    Logger::FrameworkDebug("AutotestingSystemLua::FindControl %s", path.c_str());
    
    Vector<String> controlPath;
    ParsePath(path, controlPath);
    
    return Action::FindControl(controlPath);
}
    
bool AutotestingSystemLua::SetText(const String &path, const String &text)
{
    Logger::FrameworkDebug("AutotestingSystemLua::SetText %s %s", path.c_str(), text.c_str());
    UITextField *tf = dynamic_cast<UITextField*>(FindControl(path));
    if(tf)
    {
        tf->SetText(StringToWString(text));
        return true;
    }
    return false;
}

void AutotestingSystemLua::KeyPress(int32 keyChar)
{
	UITextField *uiTextField = dynamic_cast<UITextField*>(UIControlSystem::Instance()->GetFocusedControl()); 
	if (uiTextField)
	{
		UIEvent keyPress;
		keyPress.tid = keyChar;
		keyPress.phase = UIEvent::PHASE_KEYCHAR;
		keyPress.tapCount = 1;
		keyPress.keyChar = keyChar;

		Logger::FrameworkDebug("AutotestingSystemLua::KeyPress %d phase=%d count=%d point=(%f, %f) physPoint=(%f,%f) key=%c", keyPress.tid, keyPress.phase, keyPress.tapCount, keyPress.point.x, keyPress.point.y, keyPress.physPoint.x, keyPress.physPoint.y, keyPress.keyChar);

		if (keyPress.tid == DVKEY_BACKSPACE)
		{
			//TODO: act the same way on iPhone
			WideString str = L"";
			if(uiTextField->GetDelegate()->TextFieldKeyPressed(uiTextField, (int32)uiTextField->GetText().length(), -1, str))
			{
				uiTextField->SetText(uiTextField->GetAppliedChanges((int32)uiTextField->GetText().length(),  -1, str));
			}
			
		}
		else if (keyPress.tid == DVKEY_ENTER)
		{
			uiTextField->GetDelegate()->TextFieldShouldReturn(uiTextField);
		}
		else if (keyPress.tid == DVKEY_ESCAPE)
		{
			uiTextField->GetDelegate()->TextFieldShouldCancel(uiTextField);
		}
		else if(keyPress.keyChar != 0)
		{
			WideString str;
			str += keyPress.keyChar;
			if(uiTextField->GetDelegate()->TextFieldKeyPressed(uiTextField, (int32)uiTextField->GetText().length(), 1, str))
			{
				uiTextField->SetText(uiTextField->GetAppliedChanges((int32)uiTextField->GetText().length(),  1, str));
			}
		}
	}

	/*
	UIEvent keyPress;
	keyPress.tid = keyChar;
	keyPress.phase = UIEvent::PHASE_KEYCHAR;
	keyPress.tapCount = 1;
	keyPress.keyChar = keyChar;

	Logger::FrameworkDebug("AutotestingSystemLua::KeyPress %d phase=%d count=%d point=(%f, %f) physPoint=(%f,%f) key=%c", keyPress.tid, keyPress.phase, keyPress.tapCount, keyPress.point.x, keyPress.point.y, keyPress.physPoint.x, keyPress.physPoint.y, keyPress.keyChar);

	Vector<UIEvent> emptyTouches;
	Vector<UIEvent> touches;
	touches.push_back(keyPress);
	UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
	AutotestingSystem::Instance()->OnInput(keyPress);
	*/
}

bool AutotestingSystemLua::CheckText(UIControl *control, const String &expectedText)
{
	UIStaticText *uiStaticText = dynamic_cast<UIStaticText*>(control);
	if(uiStaticText)
	{
		String actualText = WStringToString(uiStaticText->GetText());
		Log("DEBUG", Format("Compare text in control %s with expected text", uiStaticText->GetName().c_str()));
		Log("DEBUG", actualText);
		Log("DEBUG", expectedText);
		return (actualText == expectedText);
	}
	UITextField *uiTextField = dynamic_cast<UITextField*>(control);
	if(uiTextField)
	{
		String actualText = WStringToString(uiTextField->GetText());
		Log("DEBUG", Format("Compare text in control %s with expected text", uiTextField->GetName().c_str()));
		Log("DEBUG", actualText);
		Log("DEBUG", expectedText);
		return (actualText == expectedText);
	}
	return false;
}

bool AutotestingSystemLua::CheckMsgText(UIControl *control, const String &key)
{
	WideString expectedText = StringToWString(key);
	//TODO: check key in localized strings for Lua
	expectedText = autotestingLocalizationSystem->GetLocalizedString(expectedText);

	UIStaticText *uiStaticText = dynamic_cast<UIStaticText*>(control);
	if(uiStaticText)
	{
		WideString actualText = uiStaticText->GetText();
		Log("DEBUG", Format("Compare text in control %s with text by key %s", uiStaticText->GetName().c_str(), key.c_str()));
		Log("DEBUG", WStringToString(actualText));
		Log("DEBUG", WStringToString(expectedText));
		return (actualText == expectedText);
	}
	UITextField *uiTextField = dynamic_cast<UITextField*>(control);
	if(uiTextField)
	{
		WideString actualText = uiTextField->GetText();
		Log("DEBUG", Format("Compare text in control %s with text by key %s", uiTextField->GetName().c_str(), key.c_str()));
		Log("DEBUG", WStringToString(actualText));
		Log("DEBUG", WStringToString(expectedText));
		return (actualText == expectedText);
	}
	return false;
}
    
void AutotestingSystemLua::TouchDown(const Vector2 &point, int32 touchId)
{
    Logger::FrameworkDebug("AutotestingSystemLua::TouchDown point=(%f,%f) touchId=%d", point.x, point.y, touchId);
      
    UIEvent touchDown;
    touchDown.phase = UIEvent::PHASE_BEGAN;
    touchDown.tid = touchId;
    touchDown.tapCount = 1;
    UIControlSystem::Instance()->RecalculatePointToPhysical(point, touchDown.physPoint);
    UIControlSystem::Instance()->RecalculatePointToVirtual(touchDown.physPoint, touchDown.point);
        
    Action::ProcessInput(touchDown);
}
    
void AutotestingSystemLua::TouchMove(const Vector2 &point, int32 touchId)
{
    Logger::FrameworkDebug("AutotestingSystemLua::TouchMove point=(%f,%f) touchId=%d", point.x, point.y, touchId);
    
    //Logger::FrameworkDebug("TouchAction::TouchMove point=(%f, %f)", point.x, point.y);
    UIEvent touchMove;
    touchMove.tid = touchId;
    touchMove.tapCount = 1;
    UIControlSystem::Instance()->RecalculatePointToPhysical(point, touchMove.physPoint);
    UIControlSystem::Instance()->RecalculatePointToVirtual(touchMove.physPoint, touchMove.point);
    
    if(AutotestingSystem::Instance()->IsTouchDown(touchId))
    {
        touchMove.phase = UIEvent::PHASE_DRAG;
        Action::ProcessInput(touchMove);
    }
    else
    {
#ifdef __DAVAENGINE_IPHONE__
        Logger::Warning("AutotestingSystemLua::TouchMove point=(%f, %f) ignored no touch down found", point.x, point.y);
#else
        touchMove.phase = UIEvent::PHASE_MOVE;
        Action::ProcessInput(touchMove);
#endif
    }
}
    
void AutotestingSystemLua::TouchUp(int32 touchId)
{
    Logger::FrameworkDebug("AutotestingSystemLua::TouchUp touchId=%d", touchId);
    UIEvent touchUp;
    if(!AutotestingSystem::Instance()->FindTouch(touchId, touchUp))
    {
        Logger::Error("TouchAction::TouchUp touch down not found");
    }
    touchUp.phase = UIEvent::PHASE_ENDED;
    touchUp.tid = touchId;
    
    Action::ProcessInput(touchUp);
}

void AutotestingSystemLua::ParsePath(const String &path, Vector<String> &parsedPath)
{
    Logger::FrameworkDebug("AutotestingSystemLua::ParsePath path=%s", path.c_str());
    Split(path, "/", parsedPath);
}
    
bool AutotestingSystemLua::LoadWrappedLuaObjects()
{
    if(!luaState) return false; //TODO: report error?
    
	bool ret = true;
    luaopen_AutotestingSystem(luaState);	// load the wrappered module
    luaopen_UIControl(luaState);	// load the wrappered module
    luaopen_Rect(luaState);	// load the wrappered module
    luaopen_Vector(luaState);	// load the wrappered module
    luaopen_KeyedArchive(luaState);	// load the wrappered module

    if(delegate)
    {
        ret = delegate->LoadWrappedLuaObjects(luaState);
    }
    //TODO: check if modules really loaded
    return ret;
}

bool AutotestingSystemLua::LoadScript(const String &luaScript)
{
    if(!luaState) return false;
    
    if(luaL_loadstring(luaState, luaScript.c_str()) != 0)
    {
        Logger::Error("AutotestingSystemLua::LoadScript Error: unable to load %s", luaScript.c_str());
        return false;
    }
    return true;
}
    
bool AutotestingSystemLua::LoadScriptFromFile(const String &luaFilePath)
{
    if(!luaState) return false;
    
    String realPath = FilePath(luaFilePath).GetAbsolutePathname();
    if(luaL_loadfile(luaState, realPath.c_str()) != 0)
    {
        Logger::Error("AutotestingSystemLua::LoadScriptFromFile Error: unable to load %s", realPath.c_str());
        return false;
    }
    return true;
}
    
bool AutotestingSystemLua::RunScriptFromFile(const String &luaFilePath)
{
    Logger::FrameworkDebug("AutotestingSystemLua::RunScriptFromFile %s", luaFilePath.c_str());
    if(LoadScriptFromFile(luaFilePath))
    {
        return RunScript();
    }
    return false;
}
    
bool AutotestingSystemLua::RunScript(const DAVA::String &luaScript)
{
    //Logger::FrameworkDebug("AutotestingSystemLua::RunScript %s", luaScript.c_str());
    if(LoadScript(luaScript))
    {
        return RunScript();
    }
    return false;
}
    
bool AutotestingSystemLua::RunScript()
{
    //Logger::FrameworkDebug("AutotestingSystemLua::RunScript");
    lua_pcall(luaState, 0, 0, 0); //TODO: LUA_MULTRET?
    //TODO: check if lua_pcall was successfull
    return true;
}

};

#endif //__DAVAENGINE_AUTOTESTING__