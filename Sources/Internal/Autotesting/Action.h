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


#ifndef __DAVAENGINE_ACTION_H__
#define __DAVAENGINE_ACTION_H__

#include "DAVAConfig.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Base/BaseTypes.h"

#include "Utils/StringFormat.h"

#include "UI/UIControl.h"
#include "UI/UIList.h"

namespace DAVA
{

class Action : public BaseObject
{
public:
    Action();
    virtual ~Action();

    void SetName(const String &actionName);
	const String &GetName() { return name; };

    virtual void Update(float32 timeElapsed);
    virtual void Execute();
    inline bool IsExecuted() { return isExecuted; };

	void DebugLog(const String &prefix, bool toAutotestingSystem);

    static void ProcessInput(const UIEvent &input);
    
    static Vector2 FindControlPosition(const Vector<String>& controlPath);
    
    static UIControl* FindControl(const Vector<String>& controlPath);
    static UIControl* FindControl(UIControl* srcControl, const String &controlName);
    static UIControl* FindControl(UIControl* srcControl, int32 index);
    static UIControl* FindControl(UIList* srcList, int32 index);
    
    static bool IsCenterInside(UIControl* parent, UIControl* child);
    
    // helper for messages
    static String PathToString(const Vector<String>& controlPath);
    
protected:
	virtual String Dump();

    virtual bool TestCondition();

    void SetText(const Vector<String> &controlPath, const WideString &text);
    WideString GetText(const Vector<String> &controlPath);



	String name;
    bool isExecuted;
};

class KeyPressAction : public Action
{
public:
    KeyPressAction(char16 _keyChar);
    virtual ~KeyPressAction();

    virtual void Execute();
protected:
	virtual String Dump();

    void KeyPress(char16 keyChar);
    char16 keyChar;
};

class WaitAction : public Action
{
public:
    WaitAction(float32 _waitTime);
    virtual ~WaitAction();

    virtual void Execute();
    virtual void Update(float32 timeElapsed);
protected:
	virtual String Dump();

    virtual bool TestCondition();
    float32 waitTime;
};

class WaitForScreenAction : public WaitAction
{
public:
    WaitForScreenAction(const String& _controlName, float32 timeout);
    virtual ~WaitForScreenAction();

    virtual void Execute();
protected:
	virtual String Dump();

    virtual bool TestCondition();
    String screenName;
};

class WaitForUIAction : public WaitAction
{
public:
    WaitForUIAction(const String& _controlName, float32 timeout);
    WaitForUIAction(const Vector<String>& _controlName, float32 timeout);
    virtual ~WaitForUIAction();

    virtual void Execute();
protected:
	virtual String Dump();

    virtual bool TestCondition();
    Vector<String> controlPath;
};

class SetTextAction : public Action
{
public:
    SetTextAction(const String& _controlName, const WideString &_text);
    SetTextAction(const Vector<String>& _controlPath, const WideString &_text);
    virtual ~SetTextAction();

    virtual void Execute();
protected:
	virtual String Dump();

    Vector<String> controlPath;
    WideString text;
};

// class ExecuteYamlAction : public Action
// {
// public:
//     ExecuteYamlAction(const String& _yamlPath);
//     virtual ~ExecuteYamlAction();
// 
//     virtual void Load();
//     virtual void Execute();
// protected:
//     String yamlPath;
//     Deque<Action*> actions;
// };

};

#endif //__DAVAENGINE_AUTOTESTING__

#endif //__DAVAENGINE_ACTION_H__