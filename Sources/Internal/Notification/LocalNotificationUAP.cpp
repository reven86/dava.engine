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


#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Notification/LocalNotificationUAP.h"

namespace DAVA
{
using namespace Windows::Data::Xml::Dom;
using namespace Windows::UI::Notifications;

XmlDocument ^ GenerateToastDeclaration(const WideString& title, const WideString& text, bool useSound) {
    Platform::String ^ toastTitle = ref new Platform::String(title.c_str());
    Platform::String ^ toastText = ref new Platform::String(text.c_str());
    XmlDocument ^ toastXml =
    ToastNotificationManager::GetTemplateContent(ToastTemplateType::ToastText02);

    //Set the title and the text
    XmlNodeList ^ toastTextElements = toastXml->GetElementsByTagName("text");
    toastTextElements->GetAt(0)->AppendChild(toastXml->CreateTextNode(toastTitle));
    toastTextElements->GetAt(1)->AppendChild(toastXml->CreateTextNode(toastText));

    //Set silence
    if (!useSound)
    {
        IXmlNode ^ toastNode = toastXml->SelectSingleNode("/toast");
        XmlElement ^ audioNode = toastXml->CreateElement("audio");
        audioNode->SetAttribute("silent", "true");

        toastNode->AppendChild(audioNode);
    }

    return toastXml;
}

LocalNotificationUAP::LocalNotificationUAP(const String& _id)
{
    notificationId = _id;
    toastNotifier = ToastNotificationManager::CreateToastNotifier();
}

void LocalNotificationUAP::SetAction(const WideString& action)
{
}

void LocalNotificationUAP::Hide()
{
    if (!notification)
    {
        return;
    }

    toastNotifier->Hide(notification);
    notification = nullptr;
}

void LocalNotificationUAP::ShowText(const WideString& title, const WideString& text, bool useSound)
{
    XmlDocument ^ toastDoc = GenerateToastDeclaration(title, text, useSound);
    CreateOrUpdateNotification(toastDoc);
}

void LocalNotificationUAP::ShowProgress(const WideString& title,
                                        const WideString& text,
                                        uint32 total,
                                        uint32 progress,
                                        bool useSound)
{
    double percentage = (static_cast<double>(progress) / total) * 100.0;
    WideString titleText = title + Format(L" %.02f%%", percentage);
    XmlDocument ^ toastDoc = GenerateToastDeclaration(titleText, text, useSound);

    CreateOrUpdateNotification(toastDoc, nullptr, true);
}

void LocalNotificationUAP::PostDelayedNotification(const WideString& title,
                                                   const WideString& text,
                                                   int delaySeconds,
                                                   bool useSound)
{
    XmlDocument ^ toastDoc = GenerateToastDeclaration(title, text, useSound);

    Windows::Globalization::Calendar ^ calendar = ref new Windows::Globalization::Calendar;
    calendar->AddSeconds(delaySeconds);
    Windows::Foundation::DateTime dateTime = calendar->GetDateTime();

    CreateOrUpdateNotification(toastDoc, &dateTime);
}

void LocalNotificationUAP::RemoveAllDelayedNotifications()
{
    auto scheduledNotifications = toastNotifier->GetScheduledToastNotifications();

    for (unsigned i = 0; i < scheduledNotifications->Size; ++i)
    {
        toastNotifier->RemoveFromSchedule(scheduledNotifications->GetAt(i));
    }
}

void LocalNotificationUAP::CreateOrUpdateNotification(XmlDocument ^ notificationDeclaration,
                                                      const Windows::Foundation::DateTime* startTime,
                                                      bool ghostNotification)
{
    if (startTime == nullptr)
    {
        ToastNotification ^ notif = ref new ToastNotification(notificationDeclaration);
        notif->SuppressPopup = ghostNotification;
        toastNotifier->Show(notif);

        if (notification)
        {
            toastNotifier->Hide(notification);
        }
        notification = notif;
    }
    else
    {
        ScheduledToastNotification ^ notif =
        ref new ScheduledToastNotification(notificationDeclaration, *startTime);
        notif->SuppressPopup = ghostNotification;
        toastNotifier->AddToSchedule(notif);
    }
}

LocalNotificationImpl* LocalNotificationImpl::Create(const String& _id)
{
    return new LocalNotificationUAP(_id);
}

} // namespace DAVA

#endif
