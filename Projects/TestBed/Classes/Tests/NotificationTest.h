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

#ifndef __NOTIFICATION_SCREEN_H__
#define __NOTIFICATION_SCREEN_H__

#include "Infrastructure/BaseScreen.h"

using namespace DAVA;

class NotificationScreen : public BaseScreen
{
public:
    NotificationScreen();
protected:
    ~NotificationScreen() {}
public:
    void LoadResources() override;
    void UnloadResources() override;

    void Update(float32 timeElapsed) override;
    void Draw(const UIGeometricData &geometricData) override;

    void UpdateNotification();

private:
    void OnNotifyText(BaseObject *obj, void *data, void *callerData);
    void OnNotifyTextDelayed(BaseObject *obj, void *data, void *callerData);
    void OnHideText(BaseObject *obj, void *data, void *callerData);
    void OnNotifyProgress(BaseObject *obj, void *data, void *callerData);
    void OnHideProgress(BaseObject *obj, void *data, void *callerData);

    void OnNotificationTextPressed(BaseObject *obj, void *data, void *callerData);
    void OnNotificationProgressPressed(BaseObject *obj, void *data, void *callerData);

private:
    UIButton *showNotificationText;
    UIButton *showNotificationTextDelayed;
    UIButton *hideNotificationText;
    UIButton *showNotificationProgress;
    UIButton *hideNotificationProgress;

    LocalNotificationProgress *notificationProgress;
    LocalNotificationText *notificationText;

    uint32 progress;
};

#endif
