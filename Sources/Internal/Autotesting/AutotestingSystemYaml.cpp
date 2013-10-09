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


#include "Autotesting/AutotestingSystemYaml.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "AutotestingSystem.h"

#include "Utils/Utils.h"

namespace DAVA
{
    


AutotestingSystemYaml::AutotestingSystemYaml() : currentAction(NULL)
	, parsingMultitouch(NULL)
{
}

AutotestingSystemYaml::~AutotestingSystemYaml()
{
    for(Deque<Action*>::iterator it = actions.begin(); it != actions.end(); ++it)
    {
        SafeRelease(*it);
    }
    actions.clear();
}
    
void AutotestingSystemYaml::InitFromYaml(const String &yamlFilePath)
{
    YamlParser* parser = YamlParser::Create(yamlFilePath);
    if(parser)
    {
        YamlNode* rootNode = parser->GetRootNode();
        if(rootNode)
        {
            String testName = "";
            YamlNode* testNameNode = rootNode->Get("testName");
            if(testNameNode)
            {
                testName = testNameNode->AsString();
            }
            AutotestingSystem::Instance()->OnInit();//Init(testName);
            
            YamlNode* actionsNode = rootNode->Get("actions");
            AddActionsFromYamlNode(actionsNode);
            
            AutotestingSystem::Instance()->RunTests();
        }
        else
        {
            AutotestingSystem::Instance()->OnError(Format("parsing %s failed - no root node", yamlFilePath.c_str()));
        }
    }
    SafeRelease(parser);
}
    
void AutotestingSystemYaml::AddAction(Action* action)
{
    if(!AutotestingSystem::Instance()->IsInit()) return;

	if(parsingMultitouch)
	{
		TouchAction* touch = dynamic_cast<TouchAction*>(action);
		if(touch)
		{
			parsingMultitouch->AddTouch(touch);
		}
		else
		{
			AutotestingSystem::Instance()->OnError("AddAction not a touch is passed into multitouch");
		}
	}
	else if(action)
	{
		action->Retain();
		actions.push_back(action);
		action->DebugLog("AddAction", false);
	}
}

void AutotestingSystemYaml::AddActionsFromYaml(const String &yamlFilePath)
{
    if(!AutotestingSystem::Instance()->IsInit()) return;

    YamlParser* parser = YamlParser::Create(yamlFilePath);
    if(parser)
    {
        YamlNode* rootNode = parser->GetRootNode();
        if(rootNode)
        {
            YamlNode* actionsNode = rootNode->Get("actions");
            AddActionsFromYamlNode(actionsNode);
        }
        else
        {
            AutotestingSystem::Instance()->OnError(Format("parsing %s failed - no root node", yamlFilePath.c_str()));
        }
    }
    SafeRelease(parser);
}

void AutotestingSystemYaml::AddActionsFromYamlNode(YamlNode* actionsNode)
{
    if (!AutotestingSystem::Instance()->IsInit()) return;

    if(actionsNode)
    {
        Vector<YamlNode*> actionNodes = actionsNode->AsVector();
        for(uint32 i = 0; i < actionNodes.size(); ++i)
        {
            YamlNode* actionNode = actionNodes[i];
            YamlNode* actionNameNode = actionNodes[i]->Get("action");
            if(actionNode && actionNameNode)
            {                        
                String actionName = actionNameNode->AsString();
                //Logger::FrameworkDebug("AddActionsFromYamlNode action=%s", actionName.c_str());
                if(actionName == "ExecuteYaml")
                {
                    YamlNode* pathNode = actionNode->Get("path");
                    if(pathNode)
                    {
                        AddActionsFromYaml(pathNode->AsString());
                    }
                    else
                    {
                        AutotestingSystem::Instance()->OnError(Format("AddActionsFromYamlNode action %s no path", actionName.c_str()));
                    }
                }
                else if(actionName == "Click")
                {
                    YamlNode* idNode = actionNode->Get("id");
                    YamlNode* pointNode = actionNode->Get("point");
                    if(pointNode)
                    {
                        if(idNode)
                        {
                            Click(pointNode->AsVector2(), idNode->AsInt());
                        }
                        else
                        {
                            Click(pointNode->AsVector2());
                        }
                    }
                    else
                    {
                        YamlNode* controlPathNode = actionNode->Get("controlPath");
                        if(controlPathNode)
                        {
							Vector2 offset;
							YamlNode* offsetNode = actionNode->Get("offset");
							if(offsetNode)
							{
								offset = offsetNode->AsVector2();
							}

                            if(idNode)
                            {
                                Click(ParseControlPath(controlPathNode), offset, idNode->AsInt());
                            }
                            else
                            {
                                Click(ParseControlPath(controlPathNode), offset);
                            }
                        }
                        else
                        {
                            AutotestingSystem::Instance()->OnError(Format("AddActionsFromYamlNode action %s no path", actionName.c_str()));
                        }
                    }
                }
                else if(actionName == "TouchDown")
                {
                    YamlNode* idNode = actionNode->Get("id");
                    YamlNode* pointNode = actionNode->Get("point");
                    if(pointNode)
                    {
                        if(idNode)
                        {
                            TouchDown(pointNode->AsVector2(), idNode->AsInt());
                        }
                        else
                        {
                            TouchDown(pointNode->AsVector2());
                        }
                    }
                    else
                    {
                        YamlNode* controlPathNode = actionNode->Get("controlPath");
                        if(controlPathNode)
                        {
							Vector2 offset;
							YamlNode* offsetNode = actionNode->Get("offset");
							if(offsetNode)
							{
								offset = offsetNode->AsVector2();
							}

                            if(idNode)
                            {
                                TouchDown(ParseControlPath(controlPathNode), offset, idNode->AsInt());
                            }
                            else
                            {
                                TouchDown(ParseControlPath(controlPathNode), offset);
                            }
                        }
                        else
                        {
                            AutotestingSystem::Instance()->OnError(Format("AddActionsFromYamlNode action %s no path", actionName.c_str()));
                        }
                    }
                }
                else if(actionName == "TouchUp")
                {
                    YamlNode* idNode = actionNode->Get("id");
                    if(idNode)
                    {
                        TouchUp(idNode->AsInt());
                    }
                    else
                    {
                        TouchUp();
                    }
                }
                else if(actionName == "TouchMove")
                {
                    YamlNode* idNode = actionNode->Get("id");
                    YamlNode* pointNode = actionNode->Get("point");
                    YamlNode* timeNode = actionNode->Get("time");

					YamlNode* directionNode = actionNode->Get("direction");
					YamlNode* speedNode = actionNode->Get("speed");

                    if(pointNode || directionNode)
                    {
                        float32 time = 0.0f;
                        if(timeNode)
                        {
                            time = timeNode->AsFloat();
                        }

						float32 speed = 1.0f;
						if(speedNode)
						{
							speed = speedNode->AsFloat();
						}

						int32 id = 1;
						if(idNode)
						{
							id = idNode->AsInt();
						}

                        if(directionNode)
                        {
							if(idNode)
							{
								TouchMove(directionNode->AsVector2(), speed, time, idNode->AsInt());
							}
							else
							{
								TouchMove(directionNode->AsVector2(), speed, time);
							}
                        }
                        else
                        {
							if(idNode)
							{
								TouchMove(pointNode->AsVector2(), time, idNode->AsInt());
							}
							else
							{
								 TouchMove(pointNode->AsVector2(), time);
							}
                        }
                    }
					else
					{
						YamlNode* controlPathNode = actionNode->Get("controlPath");
                        if(controlPathNode)
                        {
							Vector2 offset;
							YamlNode* offsetNode = actionNode->Get("offset");
							if(offsetNode)
							{
								offset = offsetNode->AsVector2();
							}

							float32 time = 0.0f;
							if(timeNode)
							{
								time = timeNode->AsFloat();
							}

                            if(idNode)
                            {
                                TouchMove(ParseControlPath(controlPathNode), time, offset, idNode->AsInt());
                            }
                            else
                            {
                                TouchMove(ParseControlPath(controlPathNode), time, offset);
                            }
						}
						else
						{
							AutotestingSystem::Instance()->OnError(Format("AddActionsFromYamlNode action %s no point", actionName.c_str()));
						}
					}
                }
				else if(actionName == "MultiTouch")
				{
					BeginMultitouch();
					YamlNode* touchesNode = actionNode->Get("touches");
					AddActionsFromYamlNode(touchesNode);
					EndMultitouch();
				}
                else if(actionName == "SetText")
                {
                    YamlNode* controlPathNode = actionNode->Get("controlPath");
                    YamlNode* textNode = actionNode->Get("text");
                    if(controlPathNode)
                    {
                        if(textNode)
                        {
                            SetText(ParseControlPath(controlPathNode), textNode->AsWString());
                        }
                        else
                        {
                            SetText(ParseControlPath(controlPathNode), L"");
                        }
                    }
                    else
                    {
                        AutotestingSystem::Instance()->OnError(Format("AddActionsFromYamlNode action %s no path", actionName.c_str()));
                    }
                }
                else if(actionName == "Wait")
                {
                    YamlNode* timeNode = actionNode->Get("time");
                    if(timeNode)
                    {
                        Wait(timeNode->AsFloat());
                    }
                    else
                    {
                        Logger::Warning("AddActionsFromYamlNode action %s no time", actionName.c_str());
                    }
                }
                else if(actionName == "WaitForUI")
                {
                    float32 timeout = 10.0f;
                    YamlNode* timeoutNode = actionNode->Get("timeout");
                    if(timeoutNode)
                    {
                        timeout = timeoutNode->AsFloat();
                    }

                    YamlNode* controlPathNode = actionNode->Get("controlPath");
                    if(controlPathNode)
                    {
                        WaitForUI(ParseControlPath(controlPathNode), timeout);
                    }
                    else
                    {
                        AutotestingSystem::Instance()->OnError(Format("AddActionsFromYamlNode action %s no path", actionName.c_str()));
                    }
                }
				else if(actionName == "WaitForScreen")
				{
					float32 timeout = 10.0f;
                    YamlNode* timeoutNode = actionNode->Get("timeout");
                    if(timeoutNode)
                    {
                        timeout = timeoutNode->AsFloat();
                    }

                    YamlNode* screenNameNode = actionNode->Get("screenName");
                    if(screenNameNode)
                    {
                        WaitForScreen(screenNameNode->AsString(), timeout);
                    }
                    else
                    {
                        AutotestingSystem::Instance()->OnError(Format("AddActionsFromYamlNode action %s no screen name", actionName.c_str()));
                    }
				}
                else if(actionName == "KeyPress")
                {
                    YamlNode* keyNode = actionNode->Get("key");
                    if(keyNode)
                    {
                        //TODO: test conversion from int to char16
                        KeyPress((char16)keyNode->AsInt());
                    }
                    else
                    {
                        AutotestingSystem::Instance()->OnError(Format("AddActionsFromYamlNode action %s no key", actionName.c_str()));
                    }
                }
                else if(actionName == "KeyboardInput")
                {
                    YamlNode* textNode = actionNode->Get("text");
                    if(textNode)
                    {
                        KeyboardInput(textNode->AsWString());
                    }
                    else
                    {
                        AutotestingSystem::Instance()->OnError(Format("AddActionsFromYamlNode action %s no text", actionName.c_str()));
                    }
                }
                else if(actionName == "Assert")
                {
                    YamlNode* messageNode = actionNode->Get("message");
                    String messageText = "";
                    if(messageNode)
                    {
                        messageText = messageNode->AsString();
                    }

                    YamlNode* expectedNode = actionNode->Get("expected");
                    YamlNode* actualNode = actionNode->Get("actual");
                    if(expectedNode && actualNode)
                    {
                        YamlNode* expectedGetterNode = expectedNode->Get("getter");
                        YamlNode* actualGetterNode = actualNode->Get("getter");

                        if(expectedGetterNode && actualGetterNode)
                        {
                            String expectedGetterName = expectedGetterNode->AsString();
                            String actualGetterName = actualGetterNode->AsString();

                            YamlNode* expectedControlPathNode = expectedNode->Get("controlPath");
                            YamlNode* actualControlPathNode = actualNode->Get("controlPath");
                            if(expectedGetterName == "GetText")
                            {
                                if(actualGetterName == "GetText")
                                {
                                    AssertText(ParseControlPath(expectedControlPathNode), ParseControlPath(actualControlPathNode), messageText);
                                }
                                else
                                {
                                    //TODO: other supported actual getters for expected getter "GetText"
                                    AutotestingSystem::Instance()->OnError(Format("AddActionsFromYamlNode action %s wrong actual %s for expected %s", actionName.c_str(), actualGetterName.c_str(), expectedGetterName.c_str()));
                                }
                            }
                            else if(expectedGetterName == "FindControl")
                            {
                                if(actualGetterName == "FindControl")
                                {
                                    AssertBool(ParseControlPath(expectedControlPathNode), ParseControlPath(actualControlPathNode), messageText);
                                }
                                else
                                {
                                    //TODO: other supported actual getters for expected getter "FindControl"
                                    AutotestingSystem::Instance()->OnError(Format("AddActionsFromYamlNode action %s wrong actual %s for expected %s", actionName.c_str(), actualGetterName.c_str(), expectedGetterName.c_str()));
                                }
                            }
                        }
                        else if(actualGetterNode)
                        {
                            String actualGetterName = actualGetterNode->AsString();
                            YamlNode* actualControlPathNode = actualNode->Get("controlPath");
                            if(actualGetterName == "GetText")
                            {
                                AssertText(expectedNode->AsWString(), ParseControlPath(actualControlPathNode), messageText);
                            }
                            else if(actualGetterName == "FindControl")
                            {
                                AssertBool(expectedNode->AsBool(), ParseControlPath(actualControlPathNode), messageText);
                            }
                        }
                        else
                        {
                            AutotestingSystem::Instance()->OnError(Format("AddActionsFromYamlNode action %s no actual getter", actionName.c_str()));
                        }
                    }
                    else
                    {
                        AutotestingSystem::Instance()->OnError(Format("AddActionsFromYamlNode action %s no expected or actual", actionName.c_str()));
                    }
                }
                else if(actionName == "Scroll")
                {
                    float32 timeout = 10.0f;
                    YamlNode* timeoutNode = actionNode->Get("timeout");
                    if(timeoutNode)
                    {
                        timeout = timeoutNode->AsFloat();
                    }

                    int32 id = 1;
                    YamlNode* idNode = actionNode->Get("id");
                    if(idNode)
                    {
                        id = idNode->AsInt();
                    }

					Vector2 offset;
					YamlNode* offsetNode = actionNode->Get("offset");
                    if(offsetNode)
                    {
                        offset = offsetNode->AsVector2();
                    }

                    YamlNode* controlPathNode = actionNode->Get("controlPath");
                    if(controlPathNode)
                    {
                        Scroll(ParseControlPath(controlPathNode), id, timeout, offset);
                    }
                    else
                    {
                        AutotestingSystem::Instance()->OnError(Format("AddActionsFromYamlNode action %s no path", actionName.c_str()));
                    }
                }
                else
                {
                    //TODO: other actions, asserts, getters

                    AutotestingSystem::Instance()->OnError(Format("AddActionsFromYamlNode wrong action %s", actionName.c_str()));
                }
            }
            else
            {
                AutotestingSystem::Instance()->OnError("AddActionsFromYamlNode no action");
            }
        }
    }
    else
    {
        AutotestingSystem::Instance()->OnError("AddActionsFromYamlNode no actions");
    }
}

Vector<String> AutotestingSystemYaml::ParseControlPath(YamlNode* controlPathNode)
{
    Vector<String> controlPath;
    if(controlPathNode)
    {
        Vector<YamlNode*> controlPathNodes = controlPathNode->AsVector();
        if(controlPathNodes.empty())
        {
            controlPath.push_back(controlPathNode->AsString());
        }
        else
        {
            for(uint32 i = 0; i < controlPathNodes.size(); ++i)
            {
                YamlNode* controlPathPartNode = controlPathNodes[i];
                if(controlPathPartNode)
                {
                    controlPath.push_back(controlPathPartNode->AsString());
                }
                else
                {
                    AutotestingSystem::Instance()->OnError("ParseControlPath part failed");
                }
            }
        }
    }
    else
    {
        AutotestingSystem::Instance()->OnError("ParseControlPath failed");
    }
    return controlPath;
}

void AutotestingSystemYaml::Update(float32 timeElapsed)
{
    //TODO: remove all executed actions?
    if(actions.empty())
    {
        AutotestingSystem::Instance()->OnTestsFinished();
    }
    else
    {
        // executes at most one command per update
        //TODO: execute simultaneously?
        if(!currentAction)
        {
            currentAction = actions.front();
            if(currentAction)
            {
                currentAction->Execute();
            }
        }
        
        if(currentAction)
        {
            if(!currentAction->IsExecuted())
            {
                currentAction->Update(timeElapsed);
            }
            else
            {
                SafeRelease(currentAction);
                actions.pop_front();
            }
        }
        else
        {
            actions.pop_front();
        }
    }
}

void AutotestingSystemYaml::Click(const Vector2 &point, int32 id)
{
    TouchDown(point, id);
    Wait(0.05f);
    TouchUp(id);
}

void AutotestingSystemYaml::Click(const String &controlName, const Vector2 &offset, int32 id)
{
    TouchDown(controlName, offset, id);
	Wait(0.05f);
    TouchUp(id);
}

void AutotestingSystemYaml::Click(const Vector<String> &controlPath, const Vector2 &offset, int32 id)
{
    TouchDown(controlPath, offset, id);
	Wait(0.05f);
    TouchUp(id);
}

void AutotestingSystemYaml::TouchDown(const Vector2 &point, int32 id)
{
    TouchDownAction* touchDownAction = new TouchDownAction(point, id);
    AddAction(touchDownAction);
    SafeRelease(touchDownAction);
}

void AutotestingSystemYaml::TouchDown(const String &controlName, const Vector2 &offset, int32 id)
{
    TouchDownControlAction* touchDownAction = new TouchDownControlAction(controlName, offset, id);
    AddAction(touchDownAction);
    SafeRelease(touchDownAction);
}

void AutotestingSystemYaml::TouchDown(const Vector<String> &controlPath, const Vector2 &offset, int32 id)
{
    TouchDownControlAction* touchDownAction = new TouchDownControlAction(controlPath, offset, id);
    AddAction(touchDownAction);
    SafeRelease(touchDownAction);
}

void AutotestingSystemYaml::TouchUp(int32 id)
{
    TouchUpAction* touchUpAction = new TouchUpAction(id);
    AddAction(touchUpAction);
    SafeRelease(touchUpAction);
}

void AutotestingSystemYaml::TouchMove(const Vector2 &direction, float32 speed, float32 time, int32 id)
{
	TouchMoveDirAction* touchMoveDirAction = new TouchMoveDirAction(direction, speed, time, id);
    AddAction(touchMoveDirAction);
    SafeRelease(touchMoveDirAction);
}

void AutotestingSystemYaml::TouchMove(const Vector2 &point, float32 time, int32 id)
{
    TouchMoveAction* touchMoveAction = new TouchMoveAction(point, time, id);
    AddAction(touchMoveAction);
    SafeRelease(touchMoveAction);
}

void AutotestingSystemYaml::TouchMove(const String &controlName, float32 time, const Vector2 &offset, int32 id)
{
    TouchMoveControlAction* touchMoveAction = new TouchMoveControlAction(controlName, time, offset, id);
    AddAction(touchMoveAction);
    SafeRelease(touchMoveAction);
}

void AutotestingSystemYaml::TouchMove(const Vector<String> &controlPath, float32 time, const Vector2 &offset, int32 id)
{
    TouchMoveControlAction* touchMoveAction = new TouchMoveControlAction(controlPath, time, offset, id);
    AddAction(touchMoveAction);
    SafeRelease(touchMoveAction);
}

void AutotestingSystemYaml::BeginMultitouch()
{
	SafeRelease(parsingMultitouch);

	MultitouchAction* newMultitouch = new MultitouchAction();
	AddAction(newMultitouch);

	parsingMultitouch = newMultitouch;
}

void AutotestingSystemYaml::EndMultitouch()
{
	SafeRelease(parsingMultitouch);
}

void AutotestingSystemYaml::KeyPress(char16 keyChar)
{
    KeyPressAction* keyPressAction = new KeyPressAction(keyChar);
    AddAction(keyPressAction);
    SafeRelease(keyPressAction);
}

void AutotestingSystemYaml::KeyboardInput(const WideString &text)
{
    for(uint32 i = 0; i < text.size(); ++i)
    {
        KeyPress(text[i]);
    }
}

void AutotestingSystemYaml::SetText(const String &controlName, const WideString &text)
{
    SetTextAction* setTextAction = new SetTextAction(controlName, text);
    AddAction(setTextAction);
    SafeRelease(setTextAction);
}

void AutotestingSystemYaml::SetText(const Vector<String> &controlPath, const WideString &text)
{
    SetTextAction* setTextAction = new SetTextAction(controlPath, text);
    AddAction(setTextAction);
    SafeRelease(setTextAction);
}
    
void AutotestingSystemYaml::Wait(float32 time)
{
    WaitAction* waitAction = new WaitAction(time);
    AddAction(waitAction);
    SafeRelease(waitAction);
}

void AutotestingSystemYaml::WaitForScreen(const String &screenName, float32 timeout)
{
    WaitForScreenAction* waitForScreenAction = new WaitForScreenAction(screenName, timeout);
    AddAction(waitForScreenAction);
    SafeRelease(waitForScreenAction);
	Wait(0.01f); // skip first update - it can be invalid in some cases
}

void AutotestingSystemYaml::WaitForUI(const String &controlName, float32 timeout)
{
    WaitForUIAction* waitForUIAction = new WaitForUIAction(controlName, timeout);
    AddAction(waitForUIAction);
    SafeRelease(waitForUIAction);
}

void AutotestingSystemYaml::WaitForUI(const Vector<String> &controlPath, float32 timeout)
{
    WaitForUIAction* waitForUIAction = new WaitForUIAction(controlPath, timeout);
    AddAction(waitForUIAction);
    SafeRelease(waitForUIAction);
}

void AutotestingSystemYaml::Scroll(const String &controlName, int32 id, float32 timeout, const Vector2 &offset)
{
    ScrollControlAction* scrollControlAction = new ScrollControlAction(controlName, id, timeout, offset);
    AddAction(scrollControlAction);
    SafeRelease(scrollControlAction);
}

void AutotestingSystemYaml::Scroll(const Vector<String> &controlPath, int32 id, float32 timeout, const Vector2 &offset)
{
    ScrollControlAction* scrollControlAction = new ScrollControlAction(controlPath, id, timeout, offset);
    AddAction(scrollControlAction);
    SafeRelease(scrollControlAction);
}

void AutotestingSystemYaml::AssertText(const WideString &expected, const Vector<String> &controlPath, const String &assertMessage)
{
    AssertAction* assertTextAction = new AssertAction(assertMessage);
    
    VariantType expectedValue;
    expectedValue.SetWideString(expected);
    Getter* expectedGetter = new Getter(expectedValue);
    assertTextAction->SetExpectedGetter(expectedGetter);
    SafeRelease(expectedGetter);

    ControlTextGetter* actualGetter = new ControlTextGetter(controlPath);
    assertTextAction->SetActualGetter(actualGetter);
    SafeRelease(actualGetter);

    AddAction(assertTextAction);
    SafeRelease(assertTextAction);
}

void AutotestingSystemYaml::AssertText(const Vector<String> &expectedControlPath, const Vector<String> &actualControlPath, const String &assertMessage)
{
    AssertAction* assertTextAction = new AssertAction(assertMessage);
    
    ControlTextGetter* expectedGetter = new ControlTextGetter(expectedControlPath);
    assertTextAction->SetActualGetter(expectedGetter);
    SafeRelease(expectedGetter);

    ControlTextGetter* actualGetter = new ControlTextGetter(actualControlPath);
    assertTextAction->SetActualGetter(actualGetter);
    SafeRelease(actualGetter);

    AddAction(assertTextAction);
    SafeRelease(assertTextAction);
}
    
void AutotestingSystemYaml::AssertBool(bool expected, const Vector<String> &controlPath, const String &assertMessage)
{
    AssertAction* assertBoolAction = new AssertAction(assertMessage);
    
    VariantType expectedValue;
    expectedValue.SetBool(expected);
    Getter* expectedGetter = new Getter(expectedValue);
    assertBoolAction->SetExpectedGetter(expectedGetter);
    SafeRelease(expectedGetter);

    ControlBoolGetter* actualGetter = new ControlBoolGetter(controlPath);
    assertBoolAction->SetActualGetter(actualGetter);
    SafeRelease(actualGetter);

    AddAction(assertBoolAction);
    SafeRelease(assertBoolAction);
}

void AutotestingSystemYaml::AssertBool(const Vector<String> &expectedControlPath, const Vector<String> &actualControlPath, const String &assertMessage)   
{
    AssertAction* assertBoolAction = new AssertAction(assertMessage);
    
    ControlBoolGetter* expectedGetter = new ControlBoolGetter(expectedControlPath);
    assertBoolAction->SetActualGetter(expectedGetter);
    SafeRelease(expectedGetter);

    ControlBoolGetter* actualGetter = new ControlBoolGetter(actualControlPath);
    assertBoolAction->SetActualGetter(actualGetter);
    SafeRelease(actualGetter);

    AddAction(assertBoolAction);
    SafeRelease(assertBoolAction);
}

};

#endif //__DAVAENGINE_AUTOTESTING__