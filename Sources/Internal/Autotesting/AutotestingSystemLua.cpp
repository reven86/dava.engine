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

	AutotestingSystemLua::AutotestingSystemLua() : delegate(NULL), luaState(NULL)
	{
		autotestingLocalizationSystem = new LocalizationSystem();
	}

	AutotestingSystemLua::~AutotestingSystemLua()
	{
		SafeRelease(autotestingLocalizationSystem);

		if (!luaState)
		{
			return;
		}
		lua_close(luaState);
		luaState = NULL;
	}

	void AutotestingSystemLua::SetDelegate(AutotestingSystemLuaDelegate* _delegate)
	{
		delegate = _delegate;
	}

	void AutotestingSystemLua::InitFromFile(const String &luaFilePath)
	{
		if (luaState)
		{
			Logger::FrameworkDebug("AutotestingSystemLua::Has initialised already.");
			return;
		}

		Logger::FrameworkDebug("AutotestingSystemLua::InitFromFile luaFilePath=%s", luaFilePath.c_str());
		autotestingLocalizationSystem->SetDirectory("~res:/Autotesting/Strings/");
		autotestingLocalizationSystem->SetCurrentLocale(LocalizationSystem::Instance()->GetCurrentLocale());
		autotestingLocalizationSystem->Init();

		luaState = lua_open();
		luaL_openlibs(luaState);

		lua_pushcfunction(luaState, &AutotestingSystemLua::Print);
		lua_setglobal(luaState, "print");

		lua_pushcfunction(luaState, &AutotestingSystemLua::RequireModule);
		lua_setglobal(luaState, "require");

		if (!LoadWrappedLuaObjects())
		{
			AutotestingSystem::Instance()->OnError("Load wrapped lua objects was failed.");
		}

		if (!RunScriptFromFile("~res:/Autotesting/Scripts/autotesting_api.lua"))
		{
			AutotestingSystem::Instance()->OnError("Initialization of 'autotesting_api.lua' was failed.");
		}

		String setPackagePathScript = Format("SetPackagePath('~res:/Autotesting/')");
		if (!RunScript(setPackagePathScript))
		{
			AutotestingSystem::Instance()->OnError("Run of '" + setPackagePathScript + "' was failed.");
		}

		if (!LoadScriptFromFile(luaFilePath))
		{
			AutotestingSystem::Instance()->OnError("Load of '" + luaFilePath + "' was failed failed");
		}

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
		if (*path == '\0') return NULL;  /* no more templates */
		l = strchr(path, *LUA_PATHSEP);  /* find next separator */
		if (l == NULL) l = path + strlen(path);
		lua_pushlstring(L, path, l - path);  /* template */
		return l;
	}

	const char* AutotestingSystemLua::Findfile(lua_State* L, const char* name, const char* pname)
	{
		const char* path;
		name = luaL_gsub(L, name, ".", LUA_DIRSEP);
		lua_getglobal(L, "package");
		lua_getfield(L, -1, pname);
		path = lua_tostring(L, -1);
		if (path == NULL)
			luaL_error(L, LUA_QL("package.%s") " must be a string", pname);
		lua_pushliteral(L, "");  /* error accumulator */
		while ((path = Pushnexttemplate(L, path)) != NULL) {
			const char *filename;
			filename = luaL_gsub(L, lua_tostring(L, -1), LUA_PATH_MARK, name);
			lua_remove(L, -2);  /* remove path template */
			if (FileSystem::Instance()->IsFile(filename))  /* does file exist and is readable? */
				return filename;  /* return that file name */
			lua_pushfstring(L, "\n\tno file " LUA_QS, filename);
			lua_remove(L, -2);  /* remove file name */
			lua_concat(L, 2);  /* add entry to possible error message */
		}
		return NULL;  /* not found */
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
	void AutotestingSystemLua::WriteState(const String &device, const String &state)
	{
		Logger::FrameworkDebug("AutotestingSystemLua::WriteState device=%s state=%s", device.c_str(), state.c_str());
		AutotestingDB::Instance()->WriteState(device, state);
	}

	void AutotestingSystemLua::WriteCommand(const String &device, const String &state)
	{
		Logger::FrameworkDebug("AutotestingSystemLua::WriteCommand device=%s command=%s", device.c_str(), state.c_str());
		AutotestingDB::Instance()->WriteCommand(device, state);
	}

	String AutotestingSystemLua::ReadState(const String &device)
	{
		Logger::FrameworkDebug("AutotestingSystemLua::ReadState device=%s", device.c_str());
		return AutotestingDB::Instance()->ReadState(device);
	}

	String AutotestingSystemLua::ReadCommand(const String &device)
	{
		Logger::FrameworkDebug("AutotestingSystemLua::ReadCommand device=%s", device.c_str());
		return AutotestingDB::Instance()->ReadCommand(device);
	}

	void AutotestingSystemLua::InitializeDevice(const String &device)
	{
		Logger::FrameworkDebug("AutotestingSystemLua::InitializeDevice device=%s", device.c_str());
		AutotestingSystem::Instance()->InitializeDevice(device);
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
		RunScript("ResumeTest()"); //TODO: time 
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

	void AutotestingSystemLua::OnStepStart(const String &stepName)
	{
		Logger::FrameworkDebug("AutotestingSystemLua::OnStepStart %s", stepName.c_str());
		AutotestingSystem::Instance()->OnStepStart(stepName);
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
			return NULL;
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
			return NULL;
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
			return NULL;
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
		return NULL;
	}

	UIControl* AutotestingSystemLua::FindControl(UIList* srcList, int32 index)
	{
		if (UIControlSystem::Instance()->GetLockInputCounter() > 0 || !srcList)
		{
			return NULL;
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
		return NULL;
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
		keyPress.phase = UIEvent::PHASE_KEYCHAR;
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
		float32 areaSize = list->TotalAreaSize(NULL);
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
		//TODO: check key in localized strings for Lua
		expectedText = autotestingLocalizationSystem->GetLocalizedString(expectedText);

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

	void AutotestingSystemLua::TouchDown(const Vector2 &point, int32 touchId)
	{
		UIEvent touchDown;
		touchDown.phase = UIEvent::PHASE_BEGAN;
		touchDown.tid = touchId;
		touchDown.tapCount = 1;
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
			touchMove.phase = UIEvent::PHASE_DRAG;
			ProcessInput(touchMove);
		}
		else
		{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
			Logger::Warning("AutotestingSystemLua::TouchMove point=(%f, %f) ignored no touch down found", point.x, point.y);
#else
			touchMove.phase = UIEvent::PHASE_MOVE;
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
		touchUp.phase = UIEvent::PHASE_ENDED;
		touchUp.tid = touchId;

		ProcessInput(touchUp);
	}

	void AutotestingSystemLua::ProcessInput(const UIEvent &input)
	{
		Vector<UIEvent> touches;
		touches.push_back(input);
		UIControlSystem::Instance()->OnInput(0, touches, touches);

		AutotestingSystem::Instance()->OnInput(input);
	}

	inline void AutotestingSystemLua::ParsePath(const String &path, Vector<String> &parsedPath)
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
			Logger::FrameworkDebug("AutotestingSystemLua::RunScript error: %s", err);
			return false;
		}
		return true;
	}
};

#endif //__DAVAENGINE_AUTOTESTING__