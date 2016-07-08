#include "Notification/LocalNotificationText.h"

namespace DAVA
{
void LocalNotificationText::ImplShow()
{
    impl->ShowText(title, text, useSound);
}
}
