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


#include "Notification/LocalNotification.h"
#include "Utils/Utils.h"
#include "Concurrency/LockGuard.h"

namespace DAVA
{

LocalNotification::LocalNotification()
{
    impl = LocalNotificationImpl::Create(GenerateGUID());
}
    
LocalNotification::~LocalNotification()
{
    delete impl;
}

void LocalNotification::SetAction(const Message& msg)
{
	action = msg;
	impl->SetAction(L"");
}

void LocalNotification::SetTitle(const WideString &_title)
{
	if (_title != title)
	{
		isChanged = true;
		title = _title;
	}
}

void LocalNotification::SetText(const WideString &_text)
{
	if (_text != text)
	{
		isChanged = true;
		text = _text;
	}
}
    
void LocalNotification::SetUseSound(const bool value)
{
    if (useSound != value)
    {
        isChanged = true;
        useSound = value;
    }
}

void LocalNotification::Show()
{
	isVisible = true;
	isChanged = true;
}

void LocalNotification::Hide()
{
	isVisible = false;
	isChanged = true;
}

void LocalNotification::Update()
{
	if (false == isChanged)
		return;

	if (true == isVisible)
		ImplShow();
	else
		impl->Hide();

	isChanged = false;
}

}

