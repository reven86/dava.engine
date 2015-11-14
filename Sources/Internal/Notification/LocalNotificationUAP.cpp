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


#include "Notification/LocalNotificationUAP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

namespace DAVA
{

LocalNotificationUAP::LocalNotificationUAP(const String &_id)
{
	notificationId = _id;
}
    
LocalNotificationUAP::~LocalNotificationUAP()
{
}

void LocalNotificationUAP::SetAction(const WideString &action)
{
}

void LocalNotificationUAP::Hide()
{
}
void LocalNotificationUAP::ShowText(const WideString &title, const WideString &text, bool useSound)
{
}
void LocalNotificationUAP::ShowProgress(const WideString &title, const WideString &text, uint32 total, uint32 progress, bool useSound)
{
}
    
LocalNotificationImpl *LocalNotificationImpl::Create(const String &_id)
{
    return new LocalNotificationUAP(_id);
}

void LocalNotificationUAP::PostDelayedNotification(const WideString &title, const WideString &text, int delaySeconds, bool useSound)
{
}

void LocalNotificationUAP::RemoveAllDelayedNotifications()
{
}

}  // namespace DAVA

#endif
