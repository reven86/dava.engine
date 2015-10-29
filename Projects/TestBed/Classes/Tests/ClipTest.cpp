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


#include "Tests/ClipTest.h"

using namespace DAVA;

ClipTest::ClipTest ()
    : BaseScreen("ClipTest")
{
}

void ClipTest::ClipPressed(BaseObject *obj, void *data, void *callerData)
{
    enableClip = !enableClip;
    parent->SetClipContents(enableClip);
    clip->SetDebugDraw(enableClip);
}

void ClipTest::DebugDrawPressed(BaseObject *obj, void *data, void *callerData)
{
    enableDebugDraw = !enableDebugDraw;
    parent->SetDebugDraw(enableDebugDraw);
    debugDraw->SetDebugDraw(enableDebugDraw);
}

WideString ConvertToWString(const DAVA::Rect &rect)
{
    std::wstringstream str;
    str << L"x = " << rect.x << L", y = " << rect.y << L", dx = " << rect.dx << L", dy =" << rect.dy << L".";
    return str.str();
}

void ClipTest::StartPos(BaseObject *obj, void *data, void *callerData)
{
    parent->SetRect(defaultRect);
    parent->Update(0);
    startPos->SetStateText(0xFF, ConvertToWString(defaultRect));
}

void ClipTest::MoveDown(BaseObject *obj, void *data, void *callerData)
{
    DAVA::Rect rect = parent->GetRect();
    rect.y++;
    parent->SetRect(rect);
    parent->Update(0);
    startPos->SetStateText(0xFF, ConvertToWString(rect));
}

void ClipTest::MoveUp(BaseObject *obj, void *data, void *callerData)
{
    DAVA::Rect rect = parent->GetRect();
    rect.y--;
    parent->SetRect(rect);
    parent->Update(0);
    startPos->SetStateText(0xFF, ConvertToWString(rect));
}

void ClipTest::MoveRight(BaseObject *obj, void *data, void *callerData)
{
    DAVA::Rect rect = parent->GetRect();
    rect.x++;
    parent->SetRect(rect);
    parent->Update(0);
    startPos->SetStateText(0xFF, ConvertToWString(rect));
}

void ClipTest::MoveLeft(BaseObject *obj, void *data, void *callerData)
{
    DAVA::Rect rect = parent->GetRect();
    rect.x--;
    parent->SetRect(rect);
    parent->Update(0);
    startPos->SetStateText(0xFF, ConvertToWString(rect));
}

void ClipTest::LoadResources()
{
    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
    font->SetSize(13);
    float32 xShift = 0.f;
    float32 yShift = 0.f;

    Size2i screenSize = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize();
    defaultRect = DAVA::Rect(xShift, yShift, static_cast<DAVA::float32>(screenSize.dx), static_cast<DAVA::float32>(screenSize.dy));
    parent = new UIControl(defaultRect);
    parent->SetDebugDraw(enableDebugDraw);
    parent->GetBackground()->SetColor(Color(0.f, 0.5f, 0.1f, 1.f));
    parent->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    parent->SetClipContents(enableClip);
    parent->SetName("parent");
    AddControl(parent);
    //start points
    float32 startX = 0.f, startY = 0.f, w = 0.f, h = 0.f;
    w = screenSize.dx * 0.25f;
    h = screenSize.dy * 0.05f;
    startX = screenSize.dx / 2 - w / 2;
    startY = h;
    // button Clip
    clip = new UIButton(DAVA::Rect(startX, startY, w, h));
    clip->SetStateFont(0xFF, font);
    clip->SetDebugDraw(enableClip);
    clip->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    clip->SetStateText(0xFF, L"clip");
    clip->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, DAVA::Message(this, &ClipTest::ClipPressed));
    AddControl(clip);
    startY += h + h / 2;
    // button DebugDraw
    debugDraw = new UIButton(DAVA::Rect(startX, startY, w, h));
    debugDraw->SetStateFont(0xFF, font);
    debugDraw->SetDebugDraw(enableDebugDraw);
    debugDraw->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    debugDraw->SetStateText(0xFF, L"debugDraw");
    debugDraw->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, DAVA::Message(this, &ClipTest::DebugDrawPressed));
    AddControl(debugDraw);
    startY += h + h / 2;
    // button startPos
    startPos = new UIButton(DAVA::Rect(startX, startY, w, h));
    startPos->SetStateFont(0xFF, font);
    startPos->SetDebugDraw(true);
    startPos->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    startPos->SetStateText(0xFF, L"startPos");
    startPos->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, DAVA::Message(this, &ClipTest::StartPos));
    AddControl(startPos);
    startY += h + h / 2;
    // button moveLeft
    moveLeft = new UIButton(DAVA::Rect(startX, startY, w, h));
    moveLeft->SetStateFont(0xFF, font);
    moveLeft->SetDebugDraw(true);
    moveLeft->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    moveLeft->SetStateText(0xFF, L"moveLeft");
    moveLeft->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, DAVA::Message(this, &ClipTest::MoveLeft));
    AddControl(moveLeft);
    startY += h + h / 2;
    // button moveRight
    moveRight = new UIButton(DAVA::Rect(startX, startY, w, h));
    moveRight->SetStateFont(0xFF, font);
    moveRight->SetDebugDraw(true);
    moveRight->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    moveRight->SetStateText(0xFF, L"moveRight");
    moveRight->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, DAVA::Message(this, &ClipTest::MoveRight));
    AddControl(moveRight);
    startY += h + h / 2;
    // button moveUp
    moveUp = new UIButton(DAVA::Rect(startX, startY, w, h));
    moveUp->SetStateFont(0xFF, font);
    moveUp->SetDebugDraw(true);
    moveUp->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    moveUp->SetStateText(0xFF, L"moveUp");
    moveUp->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, DAVA::Message(this, &ClipTest::MoveUp));
    AddControl(moveUp);
    startY += h + h / 2;
    // button moveDown
    moveDown = new UIButton(DAVA::Rect(startX, startY, w, h));
    moveDown->SetStateFont(0xFF, font);
    moveDown->SetDebugDraw(true);
    moveDown->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    moveDown->SetStateText(0xFF, L"moveDown");
    moveDown->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, DAVA::Message(this, &ClipTest::MoveDown));
    AddControl(moveDown);

    parent->Update(0);
    BaseScreen::LoadResources();
}

void ClipTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    RemoveAllControls();
    DAVA::SafeRelease(parent);
    DAVA::SafeRelease(clip);
    DAVA::SafeRelease(debugDraw);
    DAVA::SafeRelease(startPos);
    DAVA::SafeRelease(moveLeft);
    DAVA::SafeRelease(moveRight);
    DAVA::SafeRelease(moveUp);
    DAVA::SafeRelease(moveDown);
    DAVA::SafeRelease(parent);
}

