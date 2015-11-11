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

#include "Autotesting/AutotestingSystem.h"
#include "Autotesting/AutotestingDB.h"

#include "Utils/Utils.h"
#include "Platform/DeviceInfo.h"

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "dlmalloc.h"
};

// directly include wrapped module here to compile only if __DAVAENGINE_AUTOTESTING__ is defined


extern "C" int luaopen_AutotestingSystem(lua_State *l);
extern "C" int luaopen_UIControl(lua_State *l);
extern "C" int luaopen_Rect(lua_State *l);
extern "C" int luaopen_Vector(lua_State *l);
extern "C" int luaopen_KeyedArchive(lua_State *l);
extern "C" int luaopen_Polygon2(lua_State *l);

#define LUA_OK 0

namespace DAVA
{
	static const int32 LUA_MEMORY_POOL_SIZE = 1024 * 1024 * 10;

	void* lua_allocator(void *ud, void *ptr, size_t osize, size_t nsize)
	{
		if (nsize == 0)
		{
			mspace_free(ud, ptr);
			return nullptr;
		}
		else
		{
			void* mem = mspace_realloc(ud, ptr, nsize);
			DVASSERT(mem);
			return mem;
		}
	}

	AutotestingSystemLua::AutotestingSystemLua() : delegate(nullptr), luaState(nullptr), memoryPool(nullptr), memorySpace(nullptr)
	{

	}

	AutotestingSystemLua::~AutotestingSystemLua()
	{

		if (!luaState)
		{
			return;
		}
		lua_close(luaState);
		luaState = nullptr;
    
		destroy_mspace(memorySpace);
		free(memoryPool);
	}

	void AutotestingSystemLua::SetDelegate(AutotestingSystemLuaDelegate* _delegate)
	{
		delegate = _delegate;
	}

	void AutotestingSystemLua::InitFromFile(const String &luaFilePath)
	{
		if (luaState)
		{
			Logger::Debug("AutotestingSystemLua::Has initialised already.");
			return;
		}

		Logger::Debug("AutotestingSystemLua::InitFromFile luaFilePath=%s", luaFilePath.c_str());

		memoryPool = malloc(LUA_MEMORY_POOL_SIZE);
		memset(memoryPool, 0, LUA_MEMORY_POOL_SIZE);
		memorySpace = create_mspace_with_base(memoryPool, LUA_MEMORY_POOL_SIZE, 0);
		mspace_set_footprint_limit(memorySpace, LUA_MEMORY_POOL_SIZE);
		luaState = lua_newstate(lua_allocator, memorySpace);
		luaL_openlibs(luaState);

		lua_pushcfunction(luaState, &AutotestingSystemLua::Print);
		lua_setglobal(luaState, "print");

		lua_pushcfunction(luaState, &AutotestingSystemLua::RequireModule);
		lua_setglobal(luaState, "require");

		if (!LoadWrappedLuaObjects())
		{
			AutotestingSystem::Instance()->ForceQuit("Load wrapped lua objects was failed.");
		}
        String automationAPIStrPath = AutotestingSystem::ResolvePathToAutomation("/Autotesting/Scripts/autotesting_api.lua");
		if (automationAPIStrPath.empty() || !RunScriptFromFile(automationAPIStrPath))
		{
			AutotestingSystem::Instance()->ForceQuit("Initialization of 'autotesting_api.lua' was failed.");
		}

		lua_getglobal(luaState, "SetPackagePath");
		lua_pushstring(luaState,  AutotestingSystem::ResolvePathToAutomation("/Autotesting/").c_str());
		if (lua_pcall(luaState, 1, 1, 0))
		{
			const char* err = lua_tostring(luaState, -1);
			AutotestingSystem::Instance()->ForceQuit(Format("AutotestingSystemLua::InitFromFile SetPackagePath failed: %s", err));
		}

		if (!LoadScriptFromFile(luaFilePath))
		{
			AutotestingSystem::Instance()->ForceQuit("Load of '" + luaFilePath + "' was failed failed");
		}

		lua_getglobal(luaState, "ResumeTest");
		resumeTestFunctionRef = luaL_ref(luaState, LUA_REGISTRYINDEX);

		AutotestingSystem::Instance()->OnInit();
		String baseName = FilePath(luaFilePath).GetBasename();
		lua_pushstring(luaState, baseName.c_str());
		AutotestingSystem::Instance()->RunTests();

	}

	void AutotestingSystemLua::StartTest()
	{
		RunScript();
	}

	int AutotestingSystemLua::Print(lua_State* L)
	{
		const char* str = lua_tostring(L, -1);
		Logger::Debug("AutotestingSystemLua::Print: %s", str);
		lua_pop(L, 1);
		return 0;
	}

	const char* AutotestingSystemLua::Pushnexttemplate(lua_State* L, const char* path)
	{
		const char *l;
		while (*path == *LUA_PATHSEP) path++;  /* skip separators */
		if (*path == '\0') return nullptr;  /* no more templates */
		l = strchr(path, *LUA_PATHSEP);  /* find next separator */
		if (l == nullptr) l = path + strlen(path);
		lua_pushlstring(L, path, l - path);  /* template */
		return l;
	}

	const FilePath AutotestingSystemLua::Findfile(lua_State* L, const char* name, const char* pname)
	{
		const char* path;
		name = luaL_gsub(L, name, ".", LUA_DIRSEP);
		lua_getglobal(L, "package");
		lua_getfield(L, -1, pname);
		path = lua_tostring(L, -1);
		if (path == nullptr)
			luaL_error(L, LUA_QL("package.%s") " must be a string", pname);
		lua_pushliteral(L, "");  /* error accumulator */
		FilePath filename;
		while ((path = Pushnexttemplate(L, path)) != nullptr) {
			filename = luaL_gsub(L, lua_tostring(L, -1), LUA_PATH_MARK, name);
			lua_remove(L, -2);  /* remove path template */
            if (FileSystem::Instance()->Exists(filename)) /* does file exist and is readable? */
                return filename;  /* return that file name */
			lua_pushfstring(L, "\n\tno file " LUA_QS, filename.GetAbsolutePathname().c_str());
			lua_remove(L, -2);  /* remove file name */
			lua_concat(L, 2);  /* add entry to possible error message */
		}
		return name;  /* not found */
	}

	int AutotestingSystemLua::RequireModule(lua_State* L)
	{
		String module = lua_tostring(L, -1);
		lua_pop(L, 1);
		FilePath path = Instance()->Findfile(L, module.c_str(), "path");
		if (!Instance()->LoadScriptFromFile(path)) 
		{
			AutotestingSystem::Instance()->ForceQuit("AutotestingSystemLua::RequireModule: couldn't load module " + path.GetAbsolutePathname());
		}
		lua_pushstring(Instance()->luaState, path.GetBasename().c_str());
		if (!Instance()->RunScript())
		{
			AutotestingSystem::Instance()->ForceQuit("AutotestingSystemLua::RequireModule: couldn't run module " + path.GetBasename());
		}
		lua_pushcfunction(L, lua_tocfunction(Instance()->luaState, -1));
		lua_pushstring(L, path.GetBasename().c_str());
		return 2;
	}

	void AutotestingSystemLua::StackDump(lua_State* L)
	{
		Logger::FrameworkDebug("*** Stack Dump ***");
		int i;
		int top = lua_gettop(L);

		for (i = 1; i <= top; i++) /* repeat for each level */
		{
			int t = lua_type(L, i);
			switch (t)
			{
			case LUA_TSTRING:
			{ /* strings */
				Logger::FrameworkDebug("'%s'", lua_tostring(L, i));
				break;
			}
			case LUA_TBOOLEAN:
			{ /* booleans */
				Logger::FrameworkDebug(lua_toboolean(L, i) ? "true" : "false");
				break;
			}
			case LUA_TNUMBER:
			{ /* numbers */
				Logger::FrameworkDebug("%g", lua_tonumber(L, i));
				break;
			}
			default:
			{ /* other values */
				Logger::FrameworkDebug("%s", lua_typename(L, t));
				break;
			}
			}
		}
		Logger::FrameworkDebug("*** Stack Dump END***"); /* end the listing */
	}

	// Multiplayer API
	void AutotestingSystemLua::WriteState(const String &device, const String &param, const String &state)
	{
		Logger::FrameworkDebug("AutotestingSystemLua::WriteState device=%s param=%s state=%s", device.c_str(), param.c_str(), state.c_str());
		AutotestingDB::Instance()->WriteState(device, param, state);
	}

	String AutotestingSystemLua::ReadState(const String &device, const String &param)
	{
		Logger::FrameworkDebug("AutotestingSystemLua::ReadState device=%s param=%s", device.c_str(), param.c_str());
		return AutotestingDB::Instance()->ReadState(device, param);
	}

	void AutotestingSystemLua::InitializeDevice()
	{
		AutotestingSystem::Instance()->InitializeDevice();
	}

	String AutotestingSystemLua::GetPlatform()
	{
		return DeviceInfo::GetPlatformString();
	}

	String AutotestingSystemLua::GetDeviceName()
	{
		String deviceName;
		if (DeviceInfo::GetPlatformString() == "Android")
		{
			deviceName = DeviceInfo::GetModel();
		}
		else
		{
			deviceName = WStringToString(DeviceInfo::GetName());
		}
		replace(deviceName.begin(), deviceName.end(), ' ', '_');
		replace(deviceName.begin(), deviceName.end(), '-', '_');
		return deviceName;
	}

	bool AutotestingSystemLua::IsPhoneScreen()
	{
		float32 xInch = VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx / static_cast<float32>(Core::Instance()->GetScreenDPI());
		float32 yInch = VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy / static_cast<float32>(Core::Instance()->GetScreenDPI());
		return sqrtf(xInch*xInch + yInch*yInch) <= 6.5f;
	}

	String AutotestingSystemLua::GetTestParameter(const String &parameter)
	{
		Logger::FrameworkDebug("AutotestingSystemLua::GetTestParameter parameter=%s", parameter.c_str());
		String result = AutotestingDB::Instance()->GetStringTestParameter(AutotestingSystem::Instance()->deviceName, parameter);
		Logger::FrameworkDebug("AutotestingSystemLua::GetTestParameter value=%s", result.c_str());
		return result;
	}

	void AutotestingSystemLua::Update(float32 timeElapsed)
	{
		lua_rawgeti(luaState, LUA_REGISTRYINDEX, resumeTestFunctionRef);
		if (lua_pcall(luaState, 0, 1, 0))
		{
			const char* err = lua_tostring(luaState, -1);
			Logger::Error("AutotestingSystemLua::Update error: %s", err);
		}

	}

	float32 AutotestingSystemLua::GetTimeElapsed()
	{
		return SystemTimer::FrameDelta();
	}

	void AutotestingSystemLua::OnError(const String &errorMessage)
	{
		AutotestingSystem::Instance()->OnError(errorMessage);
	}

	void AutotestingSystemLua::OnTestStart(const String &testDescription)
	{
		Logger::FrameworkDebug("AutotestingSystemLua::OnTestStart %s", testDescription.c_str());
		AutotestingSystem::Instance()->OnTestStart(testDescription);
	}

	void AutotestingSystemLua::OnTestFinished()
	{
		Logger::FrameworkDebug("AutotestingSystemLua::OnTestFinished");
		AutotestingSystem::Instance()->OnTestsFinished();
	}

	size_t AutotestingSystemLua::GetUsedMemory() const
	{
		return lua_gc(luaState, LUA_GCCOUNT, 0) * 1024 + lua_gc(luaState, LUA_GCCOUNTB, 0);
	}

	void AutotestingSystemLua::OnStepStart(const String &stepName)
	{
		AutotestingSystem::Instance()->stepIndex++;
		Logger::FrameworkDebug("AutotestingSystemLua::OnStepStart %s", stepName.c_str());
		AutotestingSystem::Instance()->OnStepStart(Format("%d %s", AutotestingSystem::Instance()->stepIndex, stepName.c_str()));
	}

	void AutotestingSystemLua::Log(const String &level, const String &message)
	{
		AutotestingDB::Instance()->Log(level, message);
	}

	void AutotestingSystemLua::WriteString(const String & name, const String & text)
	{
		Logger::FrameworkDebug("AutotestingSystemLua::WriteString name=%s text=%s", name.c_str(), text.c_str());
		AutotestingDB::Instance()->WriteString(name, text);
	}

	String AutotestingSystemLua::ReadString(const String & name)
	{
		Logger::FrameworkDebug("AutotestingSystemLua::ReadString name=%s", name.c_str());
		return AutotestingDB::Instance()->ReadString(name);
	}

	bool AutotestingSystemLua::SaveKeyedArchiveToDevice(const String &archiveName, KeyedArchive *archive)
	{
		Logger::FrameworkDebug("AutotestingSystemLua::SaveKeyedArchiveToDevice");
		return AutotestingDB::Instance()->SaveKeyedArchiveToDevice(archiveName, archive);
	}

	String AutotestingSystemLua::MakeScreenshot()
	{
		Logger::FrameworkDebug("AutotestingSystemLua::MakeScreenshot");
		AutotestingSystem::Instance()->MakeScreenShot();
		return AutotestingSystem::Instance()->GetScreenShotName();
	}

	UIControl* AutotestingSystemLua::GetScreen()
	{
		return UIControlSystem::Instance()->GetScreen();
	}

	UIControl* AutotestingSystemLua::FindControlOnPopUp(const String &path)
	{
		return FindControl(path, UIControlSystem::Instance()->GetPopupContainer());
	}

	UIControl* AutotestingSystemLua::FindControl(const String &path)
	{
		return FindControl(path, UIControlSystem::Instance()->GetScreen());
	}

	UIControl* AutotestingSystemLua::FindControl(const String &path, UIControl* srcControl)
	{
		Vector<String> controlPath;
		ParsePath(path, controlPath);

		if (UIControlSystem::Instance()->GetLockInputCounter() > 0 || !srcControl || controlPath.empty())
		{
			return nullptr;
		}

		UIControl* control = FindControl(srcControl, controlPath[0]);
		for (uint32 i = 1; i < controlPath.size(); ++i)
		{
			if (!control)
			{
				return control;
			}
			control = FindControl(control, controlPath[i]);
		}
		return control;
	}

	UIControl* AutotestingSystemLua::FindControl(UIControl* srcControl, const String &controlName)
	{
		if (UIControlSystem::Instance()->GetLockInputCounter() > 0 || !srcControl)
		{
			return nullptr;
		}
		int32 index = atoi(controlName.c_str());
		if (Format("%d", index) != controlName)
		{
			// not number
			return srcControl->FindByName(controlName);
		}
		// number
		UIList* list = dynamic_cast<UIList*>(srcControl);
		if (list)
		{
			return FindControl(list, index);
		}
		return FindControl(srcControl, index);
	}

	UIControl* AutotestingSystemLua::FindControl(UIControl* srcControl, int32 index)
	{
		if (UIControlSystem::Instance()->GetLockInputCounter() > 0 || !srcControl)
		{
			return nullptr;
		}
		const List<UIControl*> &children = srcControl->GetChildren();
		int32 childIndex = 0;
		for (List<UIControl*>::const_iterator it = children.begin(); it != children.end(); ++it, ++childIndex)
		{
			if (childIndex == index)
			{
				return (*it);
			}
		}
		return nullptr;
	}

	UIControl* AutotestingSystemLua::FindControl(UIList* srcList, int32 index)
	{
		if (UIControlSystem::Instance()->GetLockInputCounter() > 0 || !srcList)
		{
			return nullptr;
		}
		const List<UIControl*> &cells = srcList->GetVisibleCells();
		for (List<UIControl*>::const_iterator it = cells.begin(); it != cells.end(); ++it)
		{
			UIListCell* cell = dynamic_cast<UIListCell*>(*it);
			if (cell && cell->GetIndex() == index && IsCenterInside(srcList, cell))
			{
				return cell;
			}
		}
		return nullptr;
	}

	bool AutotestingSystemLua::IsCenterInside(UIControl* parent, UIControl* child)
	{
		if (!parent || !child)
		{
			return false;
		}
		const Rect &parentRect = parent->GetGeometricData().GetUnrotatedRect();
		const Rect &childRect = child->GetGeometricData().GetUnrotatedRect();
		// check if child center is inside parent rect
		return ((parentRect.x <= childRect.x + childRect.dx / 2) && (childRect.x + childRect.dx / 2 <= parentRect.x + parentRect.dx) &&
			(parentRect.y <= childRect.y + childRect.dy / 2) && (childRect.y + childRect.dy / 2 <= parentRect.y + parentRect.dy));
	}

	bool AutotestingSystemLua::SetText(const String &path, const String &text)
	{
		UITextField* tf = dynamic_cast<UITextField*>(FindControl(path));
		if (tf)
		{
			tf->SetText(StringToWString(text));
			return true;
		}
		return false;
	}

	void AutotestingSystemLua::KeyPress(int32 keyChar)
	{
		UITextField *uiTextField = dynamic_cast<UITextField*>(UIControlSystem::Instance()->GetFocusedControl());
		if (!uiTextField)
		{
			return;
		}

		UIEvent keyPress;
		keyPress.tid = keyChar;
        keyPress.phase = UIEvent::Phase::CHAR;
        keyPress.tapCount = 1;
        keyPress.keyChar = keyChar;

        Logger::FrameworkDebug("AutotestingSystemLua::KeyPress %d phase=%d count=%d point=(%f, %f) physPoint=(%f,%f) key=%c", keyPress.tid, keyPress.phase,
			keyPress.tapCount, keyPress.point.x, keyPress.point.y, keyPress.physPoint.x, keyPress.physPoint.y, keyPress.keyChar);
		switch (keyPress.tid)
		{
		case DVKEY_BACKSPACE:
		{
			//TODO: act the same way on iPhone
			WideString str = L"";
			if (uiTextField->GetDelegate()->TextFieldKeyPressed(uiTextField, static_cast<int32>(uiTextField->GetText().length()), -1, str))
			{
				uiTextField->SetText(uiTextField->GetAppliedChanges(static_cast<int32>(uiTextField->GetText().length()), -1, str));
			}
			break;
		}
		case DVKEY_ENTER:
		{
			uiTextField->GetDelegate()->TextFieldShouldReturn(uiTextField);
			break;
		}
		case DVKEY_ESCAPE:
		{
			uiTextField->GetDelegate()->TextFieldShouldCancel(uiTextField);
			break;
		}
		default:
		{
			if (keyPress.keyChar == 0)
			{
				break;
			}
			WideString str;
			str += keyPress.keyChar;
			if (uiTextField->GetDelegate()->TextFieldKeyPressed(uiTextField, static_cast<int32>(uiTextField->GetText().length()), 1, str))
			{
				uiTextField->SetText(uiTextField->GetAppliedChanges(static_cast<int32>(uiTextField->GetText().length()), 1, str));
			}
			break;
		}
		}
	}

	String AutotestingSystemLua::GetText(UIControl *control)
	{
		UIStaticText* uiStaticText = dynamic_cast<UIStaticText*>(control);
		if (uiStaticText)
		{
			return UTF8Utils::EncodeToUTF8(uiStaticText->GetText());
		}
		UITextField* uiTextField = dynamic_cast<UITextField*>(control);
		if (uiTextField)
		{
			return UTF8Utils::EncodeToUTF8(uiTextField->GetText());
		}
		return "";
	}

	bool AutotestingSystemLua::IsSelected(UIControl* control) const
	{
		Logger::Debug("AutotestingSystemLua::IsSelected Check is control %s selected", control->GetName().c_str());
		UISwitch* switchControl = dynamic_cast<UISwitch*>(control);
		if (switchControl)
		{
			return switchControl->GetIsLeftSelected();
		}
		AutotestingSystem::Instance()->OnError(Format("AutotestingSystemLua::IsSelected Couldn't get parameter for '%s'", control->GetName().c_str()));
		return false;
	}

	bool AutotestingSystemLua::IsListHorisontal(UIControl* control)
	{
		UIList* list = dynamic_cast<UIList*>(control);
		if (!list)
		{
			AutotestingSystem::Instance()->OnError("AutotestingSystemLua::Can't get UIList obj.");
		}
		return list->GetOrientation() == UIList::ORIENTATION_HORIZONTAL;
	}

	float32 AutotestingSystemLua::GetListScrollPosition(UIControl* control)
	{
		UIList* list = dynamic_cast<UIList*>(control);
		if (!list)
		{
			AutotestingSystem::Instance()->OnError("AutotestingSystemLua::Can't get UIList obj.");
		}
		float32 position = list->GetScrollPosition();
		if (position < 0)
		{
			position *= -1;
		}
		return position;
	}

	float32 AutotestingSystemLua::GetMaxListOffsetSize(UIControl* control)
	{
		UIList* list = dynamic_cast<UIList*>(control);
		if (!list)
		{
			AutotestingSystem::Instance()->OnError("AutotestingSystemLua::Can't get UIList obj.");
		}
		float32 size;
		float32 areaSize = list->TotalAreaSize(nullptr);
		Vector2 visibleSize = control->GetSize();
		if (list->GetOrientation() == UIList::ORIENTATION_HORIZONTAL)
		{
			size = areaSize - visibleSize.x;
		}
		else
		{
			size = areaSize - visibleSize.y;
		}
		return size;
	}

	Vector2 AutotestingSystemLua::GetContainerScrollPosition(UIControl* control)
	{
		UIScrollView* scrollView = dynamic_cast<UIScrollView*>(control);
		if (!scrollView)
		{
			AutotestingSystem::Instance()->OnError("AutotestingSystemLua::Can't get UIScrollView obj.");
		}
		Vector2 position = scrollView->GetScrollPosition();
		return Vector2(position.x * (-1), position.y * (-1));
	}

	Vector2 AutotestingSystemLua::GetMaxContainerOffsetSize(UIControl* control)
	{
		UIScrollView* scrollView = dynamic_cast<UIScrollView*>(control);
		if (!scrollView)
		{
			AutotestingSystem::Instance()->OnError("AutotestingSystemLua::Can't get UIScrollView obj.");
		}
		ScrollHelper* horizontalScroll = scrollView->GetHorizontalScroll();
		ScrollHelper* verticalScroll = scrollView->GetVerticalScroll();
		Vector2 totalAreaSize(horizontalScroll->GetElementSize(), verticalScroll->GetElementSize());
		Vector2 visibleAreaSize(horizontalScroll->GetViewSize(), verticalScroll->GetViewSize());
		return Vector2(totalAreaSize.x - visibleAreaSize.x, totalAreaSize.y - visibleAreaSize.y);
	}

	bool AutotestingSystemLua::CheckText(UIControl* control, const String &expectedText)
	{
		UIStaticText* uiStaticText = dynamic_cast<UIStaticText*>(control);
		if (uiStaticText)
		{
			String actualText = WStringToString(uiStaticText->GetText());
			return (actualText == expectedText);
		}
		UITextField* uiTextField = dynamic_cast<UITextField*>(control);
		if (uiTextField)
		{
			String actualText = WStringToString(uiTextField->GetText());
			return (actualText == expectedText);
		}
		return false;
	}

	bool AutotestingSystemLua::CheckMsgText(UIControl* control, const String &key)
	{
		WideString expectedText = StringToWString(key);

		UIStaticText *uiStaticText = dynamic_cast<UIStaticText*>(control);
		if (uiStaticText)
		{
			WideString actualText = uiStaticText->GetText();
			return (actualText == expectedText);
		}
		UITextField *uiTextField = dynamic_cast<UITextField*>(control);
		if (uiTextField)
		{
			WideString actualText = uiTextField->GetText();
			return (actualText == expectedText);
		}
		return false;
	}

	void AutotestingSystemLua::TouchDown(const Vector2 &point, int32 touchId, int32 tapCount)
	{
		UIEvent touchDown;
        touchDown.phase = UIEvent::Phase::BEGAN;
        touchDown.tid = touchId;
        touchDown.tapCount = tapCount;
        touchDown.physPoint = VirtualCoordinatesSystem::Instance()->ConvertVirtualToInput(point);
		touchDown.point = point;
		ProcessInput(touchDown);
	}

	void AutotestingSystemLua::TouchMove(const Vector2 &point, int32 touchId)
	{
		UIEvent touchMove;
		touchMove.tid = touchId;
		touchMove.tapCount = 1;
		touchMove.physPoint = VirtualCoordinatesSystem::Instance()->ConvertVirtualToInput(point);
		touchMove.point = point;

		if (AutotestingSystem::Instance()->IsTouchDown(touchId))
		{
            touchMove.phase = UIEvent::Phase::DRAG;
            ProcessInput(touchMove);
        }
        else
		{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
			Logger::Warning("AutotestingSystemLua::TouchMove point=(%f, %f) ignored no touch down found", point.x, point.y);
#else
            touchMove.phase = UIEvent::Phase::MOVE;
            ProcessInput(touchMove);
#endif
		}
	}

	void AutotestingSystemLua::TouchUp(int32 touchId)
	{
		UIEvent touchUp;
		if (!AutotestingSystem::Instance()->FindTouch(touchId, touchUp))
		{
			AutotestingSystem::Instance()->OnError("TouchAction::TouchUp touch down not found");
		}
        touchUp.phase = UIEvent::Phase::ENDED;
        touchUp.tid = touchId;

        ProcessInput(touchUp);
    }

	void AutotestingSystemLua::ProcessInput(const UIEvent &input)
	{
        UIEvent ev = input;
        UIControlSystem::Instance()->OnInput(&ev);

        AutotestingSystem::Instance()->OnInput(input);
    }

    inline void AutotestingSystemLua::ParsePath(const String& path, Vector<String>& parsedPath)
    {
        Split(path, "/", parsedPath);
    }

    bool AutotestingSystemLua::LoadWrappedLuaObjects()
    {
        if (!luaState)
		{
			return false; //TODO: report error?
		}

		luaopen_AutotestingSystem(luaState);	// load the wrappered module
		luaopen_UIControl(luaState);	// load the wrappered module
		luaopen_Rect(luaState);	// load the wrappered module
		luaopen_Vector(luaState);	// load the wrappered module
		luaopen_KeyedArchive(luaState);	// load the wrappered module
		luaopen_Polygon2(luaState);	// load the wrappered module

		if (!delegate)
		{
			return false;
		}
		//TODO: check if modules really loaded
		return delegate->LoadWrappedLuaObjects(luaState);
	}

	bool AutotestingSystemLua::LoadScript(const String &luaScript)
	{
		if (!luaState)
		{
			return false;
		}
		if (luaL_loadstring(luaState, luaScript.c_str()) != 0)
		{
			Logger::Error("AutotestingSystemLua::LoadScript Error: unable to load %s", luaScript.c_str());
			return false;
		}
		return true;
	}

	bool AutotestingSystemLua::LoadScriptFromFile(const FilePath &luaFilePath)
	{
		Logger::FrameworkDebug("AutotestingSystemLua::LoadScriptFromFile: %s", luaFilePath.GetAbsolutePathname().c_str());
		File* file = File::Create(luaFilePath, File::OPEN | File::READ);
		if (!file)
		{
			Logger::Error("AutotestingSystemLua::LoadScriptFromFile: couldn't open %s", luaFilePath.GetAbsolutePathname().c_str());
			return false;
		}
		char* data = new char[file->GetSize()];
		file->Read(data, file->GetSize());
		uint32 fileSize = file->GetSize();
		file->Release();
		bool result = luaL_loadbuffer(luaState, data, fileSize, luaFilePath.GetAbsolutePathname().c_str()) == LUA_OK;
		delete[] data;
		if (!result)
		{
			Logger::Error("AutotestingSystemLua::LoadScriptFromFile: couldn't load buffer %s", luaFilePath.GetAbsolutePathname().c_str());
			Logger::Error("%s", lua_tostring(luaState, -1));
			return false;
		}
		return true;
	}

	bool AutotestingSystemLua::RunScriptFromFile(const FilePath &luaFilePath)
	{
		Logger::FrameworkDebug("AutotestingSystemLua::RunScriptFromFile %s", luaFilePath.GetAbsolutePathname().c_str());
		if (LoadScriptFromFile(luaFilePath))
		{
			lua_pushstring(luaState, luaFilePath.GetBasename().c_str());
			return RunScript();
		}
		return false;
	}

	bool AutotestingSystemLua::RunScript(const String &luaScript)
	{
		if (!LoadScript(luaScript))
		{
			Logger::Error("AutotestingSystemLua::RunScript couldnt't load script %s", luaScript.c_str());
			return false;
		}
		if (lua_pcall(luaState, 0, 1, 0))
		{
			const char* err = lua_tostring(luaState, -1);
			Logger::Error("AutotestingSystemLua::RunScript error: %s", err);
			return false;
		}
		return true;
	}

	bool AutotestingSystemLua::RunScript()
	{
		if (lua_pcall(luaState, 1, 1, 0))
		{
			const char* err = lua_tostring(luaState, -1);
			Logger::Debug("AutotestingSystemLua::RunScript error: %s", err);
			return false;
		}
		return true;
	}
	int32 AutotestingSystemLua::GetServerQueueState(const String &cluster)
    {
		int32 queueState = 0;
		if (AutotestingSystem::Instance()->isDB)
		{
			RefPtr<MongodbUpdateObject> dbUpdateObject(new MongodbUpdateObject);
			KeyedArchive *clustersQueue = AutotestingDB::Instance()->FindOrInsertBuildArchive(dbUpdateObject.Get(), "clusters_queue");
			String serverName = Format("%s", cluster.c_str());
			
			if (!clustersQueue->IsKeyExists(serverName))
			{
				clustersQueue->SetInt32(serverName, 0);
			}
			else
			{
				queueState = clustersQueue->GetInt32(serverName);
			}
		}
        return queueState;
    }
    
    bool AutotestingSystemLua::SetServerQueueState(const String &cluster, int32 state)
    {
		if (!AutotestingSystem::Instance()->isDB)
		{ 
			return true;
		}
		RefPtr<MongodbUpdateObject> dbUpdateObject(new MongodbUpdateObject);
		KeyedArchive *clustersQueue = AutotestingDB::Instance()->FindOrInsertBuildArchive(dbUpdateObject.Get(), "clusters_queue");
        String serverName = Format("%s", cluster.c_str());
        bool isSet = false;
        if (!clustersQueue->IsKeyExists(serverName) || clustersQueue->GetInt32(serverName) != state)
        {
            clustersQueue->SetInt32(serverName, state);
            isSet = AutotestingDB::Instance()->SaveToDB(dbUpdateObject.Get());
        }
        return isSet;
    }
};

#endif //__DAVAENGINE_AUTOTESTING__