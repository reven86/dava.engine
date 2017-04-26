#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Notification/Private/Win10/LocalNotificationWin10.h"
#include "Utils/StringFormat.h"
#include "Utils/UTF8Utils.h"
#include "Logger/Logger.h"

namespace DAVA
{
::Windows::Data::Xml::Dom::XmlDocument ^ GenerateToastDeclaration(const WideString& title, const WideString& text, bool useSound, Platform::String ^ notificationId)
{
    using namespace ::Windows::Data::Xml::Dom;
    using namespace ::Windows::UI::Notifications;

    Platform::String ^ toastTitle = ref new Platform::String(title.c_str());
    Platform::String ^ toastText = ref new Platform::String(text.c_str());
    XmlDocument ^ toastXml = ToastNotificationManager::GetTemplateContent(ToastTemplateType::ToastText02);

    Platform::String ^ sttr = toastXml->GetXml();

    //Set the title and the text
    XmlNodeList ^ toastTextElements = toastXml->GetElementsByTagName("text");
    toastTextElements->GetAt(0)->AppendChild(toastXml->CreateTextNode(toastTitle));
    toastTextElements->GetAt(1)->AppendChild(toastXml->CreateTextNode(toastText));

    IXmlNode ^ toastNode = toastXml->SelectSingleNode("/toast");
    XmlElement ^ toastElement = static_cast<XmlElement ^>(toastNode);
    toastElement->SetAttribute("launch", notificationId);
    //Set silence
    if (!useSound)
    {
        XmlElement ^ audioNode = toastXml->CreateElement("audio");
        audioNode->SetAttribute("silent", "true");
        toastNode->AppendChild(audioNode);
    }
    return toastXml;
}

LocalNotificationUAP::LocalNotificationUAP(const String& _id)
{
    using ::Windows::UI::Notifications::ToastNotificationManager;

    notificationId = _id;
    nativeNotificationId = ref new Platform::String(UTF8Utils::EncodeToWideString(_id).c_str());
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
    using ::Windows::Data::Xml::Dom::XmlDocument;

    XmlDocument ^ toastDoc = GenerateToastDeclaration(title, text, useSound, nativeNotificationId);
    CreateOrUpdateNotification(toastDoc);
}

void LocalNotificationUAP::ShowProgress(const WideString& title,
                                        const WideString& text,
                                        uint32 total,
                                        uint32 progress,
                                        bool useSound)
{
    using ::Windows::Data::Xml::Dom::XmlDocument;

    double percentage = (static_cast<double>(progress) / total) * 100.0;
    WideString titleText = title + Format(L" %.02f%%", percentage);
    XmlDocument ^ toastDoc = GenerateToastDeclaration(titleText, text, useSound, nativeNotificationId);

    CreateOrUpdateNotification(toastDoc, 0, true);
}

void LocalNotificationUAP::PostDelayedNotification(const WideString& title,
                                                   const WideString& text,
                                                   int delaySeconds,
                                                   bool useSound)
{
    using ::Windows::Data::Xml::Dom::XmlDocument;

    XmlDocument ^ toastDoc = GenerateToastDeclaration(title, text, useSound, nativeNotificationId);

    CreateOrUpdateNotification(toastDoc, delaySeconds);
}

void LocalNotificationUAP::RemoveAllDelayedNotifications()
{
    auto scheduledNotifications = toastNotifier->GetScheduledToastNotifications();

    for (unsigned i = 0; i < scheduledNotifications->Size; ++i)
    {
        toastNotifier->RemoveFromSchedule(scheduledNotifications->GetAt(i));
    }
}

void LocalNotificationUAP::CreateOrUpdateNotification(::Windows::Data::Xml::Dom::XmlDocument ^ notificationDeclaration,
                                                      int32 delayInSeconds,
                                                      bool ghostNotification)
{
    using namespace ::Windows::UI::Notifications;

    if (delayInSeconds < 0)
    {
        DVASSERT(false, Format("Attempt to create a local notification in the past. Requested delay in seconds = %d. Ignored", delayInSeconds).c_str());
    }
    else if (delayInSeconds == 0)
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
        auto scheduledNotifications = toastNotifier->GetScheduledToastNotifications();
        if (scheduledNotifications->Size >= 4096)
        {
            DVASSERT(false, "UWP forbids scheduling more than 4096 notifications. Ignored");
            return;
        }

        Windows::Globalization::Calendar ^ calendar = ref new Windows::Globalization::Calendar;
        calendar->AddSeconds(delayInSeconds);
        Windows::Foundation::DateTime deliveryTime = calendar->GetDateTime();

        ScheduledToastNotification ^ notif = ref new ScheduledToastNotification(notificationDeclaration, deliveryTime);
        notif->SuppressPopup = ghostNotification;

        try
        {
            toastNotifier->AddToSchedule(notif);
        }
        catch (Platform::Exception ^ e)
        {
            Logger::Error("Exception occured when tried to schedule a notification: %s (hresult=0x%08X)", UTF8Utils::EncodeToUTF8(e->Message->Data()).c_str(), e->HResult);
        }
    }
}

LocalNotificationImpl* LocalNotificationImpl::Create(const String& _id)
{
    return new LocalNotificationUAP(_id);
}

void LocalNotificationImpl::RequestPermissions()
{
}

} // namespace DAVA

#endif
