#pragma once

#include "Base/BaseTypes.h"

#include "Notification/Private/Mac/NativeDelegateMac.h"
#include "Notification/Private/Ios/NativeDelegateIos.h"
#include "Notification/Private/Win10/NativeDelegateWin10.h"
#include "Notification/Private/NativeDelegateNotImplemented.h"

namespace DAVA
{
class LocalNotificationController;

class NativeDelegate : public NativeDelegateImpl
{
public:
    NativeDelegate(LocalNotificationController& controller);
};
}
