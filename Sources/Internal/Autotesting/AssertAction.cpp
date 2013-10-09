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


#include "Autotesting/AssertAction.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Autotesting/AutotestingSystem.h"

namespace DAVA
{
    
Getter::Getter()
{
    SetName("Getter");
}

Getter::Getter(const VariantType &_value) : Action(), value(_value)
{
    SetName("Getter");
}

String Getter::Dump()
{
	String baseStr = Action::Dump();
	return Format("%s type=%d", baseStr.c_str(), value.type);
}

//-----------------------------------------------------------------

ControlGetter::ControlGetter(const Vector<String> &_controlPath) : Getter(), controlPath(_controlPath)
{
    SetName("ControlGetter");
}
    
const VariantType &ControlGetter::Get()
{
    if(!isExecuted)
    {
        Execute();
    }
    return Getter::Get();
}

String ControlGetter::Dump()
{
	String baseStr = Getter::Dump();
	String controlPathStr = PathToString(controlPath);
	return Format("%s controlPath=%s", baseStr.c_str(), controlPathStr.c_str());
}

//-----------------------------------------------------------------
ControlTextGetter::ControlTextGetter(const Vector<String> &_controlPath) : ControlGetter(_controlPath)
{
    SetName("ControlTextGetter");
}
    
void ControlTextGetter::Execute()
{
    value.SetWideString(GetText(controlPath));
    ControlGetter::Execute();
}

//-----------------------------------------------------------------

ControlBoolGetter::ControlBoolGetter(const Vector<String> &_controlPath) : ControlGetter(_controlPath)
{
    SetName("ControlBoolGetter");
};
    
void ControlBoolGetter::Execute()
{
    value.SetBool(FindControl(controlPath) != NULL);
    ControlGetter::Execute();
}

//-----------------------------------------------------------------

AssertAction::AssertAction(const String &_message) : Action()
    , expected(NULL)
    , actual(NULL)
    , message(_message)
{
    SetName("AssertAction");    
}

AssertAction::~AssertAction()
{
    SafeRelease(expected); 
    SafeRelease(actual);
}

void AssertAction::SetExpectedGetter(Getter *_expected)
{
    SafeRelease(expected);
    expected = SafeRetain(_expected);
}
void AssertAction::SetActualGetter(Getter *_actual)
{
    SafeRelease(actual);
    actual = SafeRetain(_actual);
}

void AssertAction::Execute()
{
    bool isPassed = false;
    if(actual && expected)
    {
        isPassed = (actual->Get() == expected->Get());
    }
	Action::Execute();
    AutotestingSystem::Instance()->OnTestStep(message, isPassed);
}

String AssertAction::Dump()
{
	String baseStr = Action::Dump();
	return Format("%s message=%s", baseStr.c_str(), message.c_str());
}

};

#endif //__DAVAENGINE_AUTOTESTING__