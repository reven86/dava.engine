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


#ifndef __DAVAENGINE_AUTOTESTING_SYSTEM_LUA_H__
#define __DAVAENGINE_AUTOTESTING_SYSTEM_LUA_H__

#include "DAVAConfig.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Math/Vector.h"
#include "Base/Singleton.h"

#include "UI/UIControl.h"
#include "UI/UIList.h"

#include "FileSystem/LocalizationSystem.h"
#include "Autotesting/AutotestingSystem.h"

struct lua_State;

namespace DAVA
{
    
#if !defined(SWIG)
class AutotestingSystemLuaDelegate
{
public:
	virtual ~AutotestingSystemLuaDelegate() {}

	virtual bool LoadWrappedLuaObjects(lua_State* luaState) = 0;
};
#endif //SWIG

class AutotestingSystemLua : public Singleton<AutotestingSystemLua>
{
public:
    AutotestingSystemLua();
    ~AutotestingSystemLua();
    
#if !defined(SWIG)
    void SetDelegate(AutotestingSystemLuaDelegate* _delegate);

    void InitFromFile(const String &luaFilePath);
    
    void StartTest();

    void Update(float32 timeElapsed);

	static int Print(lua_State* L);
	static int RequireModule(lua_State* L);

	static void StackDump(lua_State* L);
	const char *Pushnexttemplate (lua_State* L, const char* path);
	const FilePath Findfile (lua_State* L, const char* name, const char* pname);
#endif //SWIG
    
    // autotesting system api   
    void OnError(const String &errorMessage);
    void OnTestFinished();
    
    size_t GetUsedMemory() const;
    
    float32 GetTimeElapsed();
    
	// Test organization API
	void OnTestStart(const String &testName);

	void OnStepStart(const String &stepName);

	void Log(const String &level, const String &message);
	
    // autotesting api
    UIControl *GetScreen();
    UIControl *FindControl(const String &path);
	UIControl* FindControl(const String &path, UIControl* screen);
	UIControl* FindControlOnPopUp(const String &path);
	UIControl* FindControl(UIControl* srcControl, const String &controlName);
	UIControl* FindControl(UIControl* srcControl, int32 index);
	UIControl* FindControl(UIList* srcList, int32 index);

	bool IsCenterInside(UIControl* parent, UIControl* child);
	bool IsSelected(UIControl* control) const;

	bool IsListHorisontal(UIControl* control);
	float32 GetListScrollPosition(UIControl* control);
	float32 GetMaxListOffsetSize(UIControl* control);

	Vector2 GetContainerScrollPosition(UIControl* control);
	Vector2 GetMaxContainerOffsetSize(UIControl* control);

	void TouchDown(const Vector2 &point, int32 touchId, int32 tapCount);
    void TouchMove(const Vector2 &point, int32 touchId);
    void TouchUp(int32 touchId);
    
	// Keyboard action
	void KeyPress(int32 keyChar);

	void ProcessInput(const UIEvent &input);

    // helpers
    bool SetText(const String &path, const String &text); // lua uses ansi strings
    bool CheckText(UIControl* control, const String &expectedText);
    bool CheckMsgText(UIControl* control, const String &key);
	String GetText(UIControl* control);

	// multiplayer api
	void WriteState(const String &device, const String &state);
	void WriteCommand(const String &device, const String &state);
    int32 GetServerQueueState(const String &serverName);
    bool SetServerQueueState(const String &serverName, int32 state);


	String ReadState(const String &device);
	String ReadCommand(const String &device);

	void InitializeDevice(const String &device);

	String GetDeviceName();
	String GetPlatform();

	bool IsPhoneScreen();

	// DB storing
	bool SaveKeyedArchiveToDevice(const String &archiveName, KeyedArchive* archive);

	String GetTestParameter(const String &device);

	void WriteString(const String &name, const String &text);
	String ReadString(const String &name);

	String MakeScreenshot();

protected:
#if !defined(SWIG)
    inline void ParsePath(const String &path, Vector<String> &parsedPath);
    
    bool LoadScript(const String &luaScript);
    bool LoadScriptFromFile(const FilePath &luaFilePath);
    bool RunScript();
    
    bool RunScript(const String &luaScript);
    bool RunScriptFromFile(const FilePath &luaFilePath);
    bool LoadWrappedLuaObjects();
	

	AutotestingSystemLuaDelegate* delegate;
    lua_State* luaState; //TODO: multiple lua states
    
#endif //SWIG
private:
    void* memoryPool;
    void* memorySpace;
    int resumeTestFunctionRef;
};

};

#endif //__DAVAENGINE_AUTOTESTING__

#endif //__DAVAENGINE_AUTOTESTING_SYSTEM_LUA_H__