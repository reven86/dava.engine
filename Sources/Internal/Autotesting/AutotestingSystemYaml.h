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


#ifndef __DAVAENGINE_AUTOTESTING_SYSTEM_YAML_H__
#define __DAVAENGINE_AUTOTESTING_SYSTEM_YAML_H__

#include "DAVAConfig.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Autotesting/Action.h"
#include "Autotesting/TouchAction.h"
#include "Autotesting/AssertAction.h"

#include "Base/Singleton.h"

namespace DAVA
{

class AutotestingSystemYaml : public Singleton<AutotestingSystemYaml>
{
public:
    AutotestingSystemYaml();
    ~AutotestingSystemYaml();

    void InitFromYaml(const String &yamlFilePath);

    void AddAction(Action* action);
    void AddActionsFromYaml(const String &yamlFilePath);

    void Update(float32 timeElapsed);

    // API (high level)
    void Click(const Vector2 &point, int32 id = 1);
    void Click(const String &controlName, const Vector2 &offset = Vector2(), int32 id = 1);
    void Click(const Vector<String> &controlPath, const Vector2 &offset = Vector2(), int32 id = 1);

    void TouchDown(const Vector2 &point, int32 id = 1);
    void TouchDown(const String &controlName, const Vector2 &offset = Vector2(), int32 id = 1);
    void TouchDown(const Vector<String> &controlPath, const Vector2 &offset = Vector2(), int32 id = 1);

    void TouchUp(int32 id = 1);

	void TouchMove(const Vector2 &direction, float32 speed, float32 time, int32 id = 1);

    void TouchMove(const Vector2 &point, float32 time, int32 id = 1);
    void TouchMove(const String &controlName, float32 time, const Vector2 &offset = Vector2(), int32 id = 1);
    void TouchMove(const Vector<String> &controlPath, float32 time, const Vector2 &offset = Vector2(), int32 id = 1);

	void BeginMultitouch();
	void EndMultitouch();

    void KeyPress(char16 keyChar);
    void KeyboardInput(const WideString &text);

    void SetText(const String &controlName, const WideString &text);
    void SetText(const Vector<String> &controlPath, const WideString &text);
    
    void Wait(float32 time);

	void WaitForScreen(const String &screenName, float32 timeout = 10.0f);

    void WaitForUI(const String &controlName, float32 timeout = 10.0f);
    void WaitForUI(const Vector<String> &controlPath, float32 timeout = 10.0f);

    void Scroll(const String &controlName, int32 id = 1, float32 timeout = 10.0f, const Vector2 &offset = Vector2());
    void Scroll(const Vector<String> &controlPath, int32 id = 1, float32 timeout = 10.0f, const Vector2 &offset = Vector2());

    void AssertText(const WideString &expected, const Vector<String> &controlPath, const String &assertMessage = "");
    void AssertText(const Vector<String> &expectedControlPath, const Vector<String> &actualControlPath, const String &assertMessage = "");
    
    void AssertBool(bool expected, const Vector<String> &controlPath, const String &assertMessage = "");
    void AssertBool(const Vector<String> &expectedControlPath, const Vector<String> &actualControlPath, const String &assertMessage = "");    

protected:
    void AddActionsFromYamlNode(YamlNode* actionsNode);
    Vector<String> ParseControlPath(YamlNode* controlPathNode);

	MultitouchAction* parsingMultitouch;

    Action* currentAction;
    Deque<Action*> actions;
};

};

#endif //__DAVAENGINE_AUTOTESTING__

#endif //__DAVAENGINE_AUTOTESTING_SYSTEM_YAML_H__