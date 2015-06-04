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


#include "UISpinnerMetadata.h"
#include "StringUtils.h"

namespace DAVA {

UISpinnerMetadata::UISpinnerMetadata(QObject* parent) :
UIControlMetadata(parent)
{
}

void UISpinnerMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
	UIControlMetadata::InitializeControl(controlName, position);

	int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        UISpinner* spinner = static_cast<UISpinner*>(this->treeNodeParams[i].GetUIControl());
		if (spinner->GetButtonNext() && spinner->GetButtonPrevious())
		{
			spinner->GetButtonNext()->SetStateText(0, L"Next");
			spinner->GetButtonPrevious()->SetStateText(0, L"Prev");
			
			// Define some initial positions for UISpinner buttons.
			PositionSpinnerButtons(spinner);
		}
    }
}

QString UISpinnerMetadata::GetPrevButtonText()
{
	if (!VerifyActiveParamID() || !GetPrevButton())
    {
        return QString();
    }

	UIStaticText* textControl = GetPrevButton()->GetStateTextControl(this->uiControlStates[GetActiveStateIndex()]);
	if (textControl)
	{
		return WideString2QString(textControl->GetText());
	}
	
	return QString();
}

void UISpinnerMetadata::SetPrevButtonText(const QString& value)
{
	if (!VerifyActiveParamID() || !GetPrevButton())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		GetPrevButton()->SetStateText(this->uiControlStates[i], QStrint2WideString(value));
	}
}
	
QString UISpinnerMetadata::GetNextButtonText()
{
	if (!VerifyActiveParamID() || !GetNextButton())
    {
        return QString();
    }
	
	UIStaticText* textControl = GetNextButton()->GetStateTextControl(this->uiControlStates[GetActiveStateIndex()]);
	if (textControl)
	{
		return WideString2QString(textControl->GetText());
	}
	
	return QString();
}

void UISpinnerMetadata::SetNextButtonText(const QString& value)
{
	if (!VerifyActiveParamID() || !GetNextButton())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		GetNextButton()->SetStateText(this->uiControlStates[i], QStrint2WideString(value));
	}
}

UISpinner* UISpinnerMetadata::GetActiveUISpinner()
{
    return static_cast<UISpinner*>(GetActiveUIControl());
}

UIButton* UISpinnerMetadata::GetPrevButton()
{
	UISpinner* activeSpinner = GetActiveUISpinner();
	if (!activeSpinner)
	{
		return NULL;
	}
	
	return activeSpinner->GetButtonPrevious();
}
	
UIButton* UISpinnerMetadata::GetNextButton()
{
	UISpinner* activeSpinner = GetActiveUISpinner();
	if (!activeSpinner)
	{
		return NULL;
	}
	
	return activeSpinner->GetButtonNext();
}

void UISpinnerMetadata::PositionSpinnerButtons(UISpinner* spinner)
{
	if (!spinner || !spinner->GetButtonPrevious() || !spinner->GetButtonNext())
	{
		return;
	}

	static const float32 RELATIVE_SPINNER_BUTTONS_WIDTH = 0.25f;
	
	// Position Prev and Next buttons on the right side of the control.
	Rect controlRect = spinner->GetRect();
	Rect newPrevButtonRect;
	newPrevButtonRect.x = (controlRect.dx * (1.0f - RELATIVE_SPINNER_BUTTONS_WIDTH));
	newPrevButtonRect.y = 0;
	newPrevButtonRect.dx = controlRect.dx * RELATIVE_SPINNER_BUTTONS_WIDTH;
	newPrevButtonRect.dy = controlRect.dy / 2;
	spinner->GetButtonPrevious()->SetRect(newPrevButtonRect);

	// The rect for "Next" button is the same, but shifted down.
	Rect newNextButtonRect = newPrevButtonRect;
	newNextButtonRect.y = controlRect.dy / 2;
	spinner->GetButtonNext()->SetRect(newNextButtonRect);
}

};