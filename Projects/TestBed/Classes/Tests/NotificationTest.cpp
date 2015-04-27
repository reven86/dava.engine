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


#include "NotificationTest.h"
#include "Base/Message.h"

using namespace DAVA;

NotificationScreen::NotificationScreen()
    : UITestTemplate<NotificationScreen>("NotificationScreen")
    , showNotificationText(NULL)
	, showNotificationProgress(NULL)
	, hideNotificationProgress(NULL)
	, returnButton(NULL)
	, notificationProgress(NULL)
	, notificationText(NULL)
	, progress(0)
{
}

void NotificationScreen::LoadResources()
{
	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
	DVASSERT(font);

	font->SetSize(30);

	showNotificationText = new UIButton(Rect(10, 10, 450, 60));
	showNotificationText->SetStateFont(0xFF, font);
	showNotificationText->SetStateFontColor(0xFF, Color::White);
	showNotificationText->SetStateText(0xFF, L"Notify text");

	showNotificationText->SetDebugDraw(true);
	showNotificationText->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NotificationScreen::OnNotifyText));
	AddControl(showNotificationText);

	hideNotificationText = new UIButton(Rect(10, 100, 450, 60));
	hideNotificationText->SetStateFont(0xFF, font);
	hideNotificationText->SetStateFontColor(0xFF, Color::White);
	hideNotificationText->SetStateText(0xFF, L"Hide text");

	hideNotificationText->SetDebugDraw(false);
	hideNotificationText->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NotificationScreen::OnHideText));
	AddControl(hideNotificationText);

	showNotificationProgress = new UIButton(Rect(500, 10, 450, 60));
	showNotificationProgress->SetStateFont(0xFF, font);
	showNotificationProgress->SetStateFontColor(0xFF, Color::White);
	showNotificationProgress->SetStateText(0xFF, L"Notify progress");

	showNotificationProgress->SetDebugDraw(true);
	showNotificationProgress->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NotificationScreen::OnNotifyProgress));
	AddControl(showNotificationProgress);

	hideNotificationProgress = new UIButton(Rect(500, 100, 450, 60));
	hideNotificationProgress->SetStateFont(0xFF, font);
	hideNotificationProgress->SetStateFontColor(0xFF, Color::White);
	hideNotificationProgress->SetStateText(0xFF, L"Hide progress");

	hideNotificationProgress->SetDebugDraw(false);
	hideNotificationProgress->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NotificationScreen::OnHideProgress));
	AddControl(hideNotificationProgress);

	returnButton = new UIButton(Rect(10, 410, 450, 60));
	returnButton->SetStateFont(0xFF, font);
	returnButton->SetStateFontColor(0xFF, Color::White);
	returnButton->SetStateText(0xFF, L"Back to test list");

	returnButton->SetDebugDraw(true);
	returnButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NotificationScreen::ReturnBack));
	AddControl(returnButton);


	SafeRelease(font);
}

void NotificationScreen::UnloadResources()
{
    RemoveAllControls();
    SafeRelease(returnButton);
    SafeRelease(showNotificationText);
    SafeRelease(showNotificationProgress);
    SafeRelease(hideNotificationProgress);
}

void NotificationScreen::WillAppear()
{ 
}

void NotificationScreen::WillDisappear()
{ }


void NotificationScreen::Update(float32 timeElapsed)
{
	if (NULL == notificationProgress)
		return;

	static float32 timeCounter = 0;
	timeCounter += timeElapsed;

	if (0.25 <= timeCounter)
	{
		timeCounter = 0;

		if (100 == progress)
		{
			progress = 0;
		}

		notificationProgress->SetProgressCurrent(progress++);
	}
}

void NotificationScreen::Draw(const UIGeometricData &geometricData)
{
	RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void NotificationScreen::UpdateNotification()
{
	if (NULL == notificationProgress)
		return;

	if (100 == progress)
	{
		progress = 0;
	}

	notificationProgress->SetProgressCurrent(progress++);
}

void NotificationScreen::ReturnBack(BaseObject *obj, void *data, void *callerData)
{
    if (nullptr != notificationProgress)
    {
        notificationProgress->Hide();
    }
    if (nullptr != notificationText)
    {
        notificationText->Hide();
    }
//	UIScreenManager::Instance()->SetScreen(SCREEN_START);
    isFinished = true;
}

void NotificationScreen::OnNotifyText(BaseObject *obj, void *data, void *callerData)
{
	if (NULL == notificationText)
	{
		notificationText = LocalNotificationController::Instance()->CreateNotificationText();
        notificationText->Update();

		notificationText->SetAction(Message(this, &NotificationScreen::OnNotificationTextPressed));
	}
	else
	{
		notificationText->Show();
	}

	notificationText->SetTitle(L"Application is on foreground!");
	notificationText->SetText(L"This text appeared at button press ");

	hideNotificationText->SetDebugDraw(true);
}

void NotificationScreen::OnHideText(BaseObject *obj, void *data, void *callerData)
{
	if (notificationText && notificationText->IsVisible())
	{
		notificationText->Hide();
		hideNotificationText->SetDebugDraw(false);
	}
}

void NotificationScreen::OnNotifyProgress(BaseObject *obj, void *data, void *callerData)
{
	if (NULL == notificationProgress)
	{
		notificationProgress = LocalNotificationController::Instance()->CreateNotificationProgress(L"", L"", 100, 0);
		notificationProgress->SetAction(Message(this, &NotificationScreen::OnNotificationProgressPressed));
	}
	else
	{
		notificationProgress->Show();
	}

	notificationProgress->SetTitle(L"Fake Download Progress");
	notificationProgress->SetText(L"You pressed the button");

	hideNotificationProgress->SetDebugDraw(true);
}

void NotificationScreen::OnHideProgress(BaseObject *obj, void *data, void *callerData)
{
	if (notificationProgress && notificationProgress->IsVisible())
	{
		notificationProgress->Hide();
		hideNotificationProgress->SetDebugDraw(false);
	}
}

void NotificationScreen::OnNotificationTextPressed(BaseObject *obj, void *data, void *callerData)
{

}

void NotificationScreen::OnNotificationProgressPressed(BaseObject *obj, void *data, void *callerData)
{
}
