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


#include "IconHelper.h"

QString IconHelper::GetIconPathForClassName(const QString &className)
{
	QString s = ":/Icons/" + className.toLower() + ".png";

	return s;
}

QString IconHelper::GetIconPathForUIControl(DAVA::UIControl *uiControl)
{
	QString className = "UIControl";
	if (!uiControl->GetCustomControlType().empty())
	{
		className = "UICustomControl";
	}
	else if (dynamic_cast<UIButton*>(uiControl))
	{
		className = "UIButton";
	}
	else if (dynamic_cast<UIList*>(uiControl))
	{
		className = "UIList";
	}
	else if (dynamic_cast<UIScrollBar*>(uiControl))
	{
		className = "UIScrollBar";
	}
	else if (dynamic_cast<UIScrollView*>(uiControl))
	{
		className = "UIScrollView";
	}
	else if (dynamic_cast<UISlider*>(uiControl))
	{
		className = "UISlider";
	}
	else if (dynamic_cast<UISpinner*>(uiControl))
	{
		className = "UISpinner";
	}
	else if (dynamic_cast<UIStaticText*>(uiControl))
	{
		className = "UIStaticText";
	}
	else if (dynamic_cast<UISwitch*>(uiControl))
	{
		className = "UISwitch";
	}
	else if (dynamic_cast<UITextField*>(uiControl))
	{
		className = "UITextField";
	}
	else if (dynamic_cast<UIAggregatorControl*>(uiControl))
	{
		className = "UIAggregatorControl";
	}

	return GetIconPathForClassName(className);
}

QString IconHelper::GetPlatformIconPath()
{
	return ":/Icons/079i.png";
}

QString IconHelper::GetScreenIconPath()
{
	return ":/Icons/068i.png";
}

QString IconHelper::GetAggregatorIconPath()
{
	return ":/Icons/170.png";
}

QString IconHelper::GetIgnoreIconPath()
{
	return ":/Icons/101.png";
}
