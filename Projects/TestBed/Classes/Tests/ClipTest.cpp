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
    clip->SetDebugDraw(enableClip);

    fullSizeWgt->SetClipContents(enableClip);
    parent1->SetClipContents(enableClip);
    parent2->SetClipContents(enableClip);
    child1->SetClipContents(enableClip);
    child2->SetClipContents(enableClip);
}

void ClipTest::DebugDrawPressed(BaseObject *obj, void *data, void *callerData)
{
    enableDebugDraw = !enableDebugDraw;
    debugDraw->SetDebugDraw(enableDebugDraw);

    fullSizeWgt->SetDebugDraw(enableDebugDraw);
    parent1->SetDebugDraw(enableDebugDraw);
    parent2->SetDebugDraw(enableDebugDraw);
    child1->SetDebugDraw(enableDebugDraw);
    child2->SetDebugDraw(enableDebugDraw);
}

WideString ConvertToWString(const Rect& rect)
{
    std::wstringstream str;
    str << L"x = " << rect.x << L", y = " << rect.y << L", dx = " << rect.dx << L", dy =" << rect.dy << L".";
    return str.str();
}

void ClipTest::StartPos(BaseObject *obj, void *data, void *callerData)
{
    fullSizeWgt->SetRect(defaultRect);
    fullSizeWgt->Update(0);
    startPos->SetStateText(0xFF, ConvertToWString(defaultRect));
}

void ClipTest::MoveDown(BaseObject *obj, void *data, void *callerData)
{
    Rect rect = fullSizeWgt->GetRect();
    rect.y++;
    fullSizeWgt->SetRect(rect);
    fullSizeWgt->Update(0);
    startPos->SetStateText(0xFF, ConvertToWString(rect));
}

void ClipTest::MoveUp(BaseObject *obj, void *data, void *callerData)
{
    Rect rect = fullSizeWgt->GetRect();
    rect.y--;
    fullSizeWgt->SetRect(rect);
    fullSizeWgt->Update(0);
    startPos->SetStateText(0xFF, ConvertToWString(rect));
}

void ClipTest::MoveRight(BaseObject *obj, void *data, void *callerData)
{
    Rect rect = fullSizeWgt->GetRect();
    rect.x++;
    fullSizeWgt->SetRect(rect);
    fullSizeWgt->Update(0);
    startPos->SetStateText(0xFF, ConvertToWString(rect));
}

void ClipTest::MoveLeft(BaseObject *obj, void *data, void *callerData)
{
    Rect rect = fullSizeWgt->GetRect();
    rect.x--;
    fullSizeWgt->SetRect(rect);
    fullSizeWgt->Update(0);
    startPos->SetStateText(0xFF, ConvertToWString(rect));
}

void ClipTest::LoadResources()
{
    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
    font->SetSize(13);
    //start points
    Size2i screenSize = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize();
    float32 startX = 0.f, startY = 0.f, w = 0.f, h = 0.f;
    w = screenSize.dx * 0.25f;
    h = screenSize.dy * 0.05f;
    startX = screenSize.dx * 0.5f - w * 0.5f;
    startY = h;
    //full-size widget
    defaultRect = Rect(0.f, 0.f, static_cast<float32>(screenSize.dx), static_cast<float32>(screenSize.dy));
    fullSizeWgt = new UIControl(defaultRect);
    fullSizeWgt->SetDebugDraw(enableDebugDraw);
    fullSizeWgt->GetBackground()->SetColor(Color(0.f, 0.5f, 0.1f, 1.f));
    fullSizeWgt->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    fullSizeWgt->SetClipContents(enableClip);
    fullSizeWgt->SetName("fullSizeWgt");
    AddControl(fullSizeWgt);

    float32 parentX(0.05f * startX), parentY(parentX), parentW(0.6f * startX), parentH(parentW);
    parent1 = new UIControl(Rect(parentX, parentY, parentW, parentH));
    parent1->SetDebugDraw(enableDebugDraw);
    parent1->GetBackground()->SetColor(Color(0.5f, 0.f, 0.1f, 1.f));
    parent1->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    parent1->SetClipContents(enableClip);
    parent1->SetName("parent1");
    child1 = new UIControl(Rect(parentW * 0.5f, parentH * 0.5f, parentW, parentH));
    child1->SetDebugDraw(enableDebugDraw);
    child1->GetBackground()->SetColor(Color(0.1f, 0.f, 0.5f, 1.f));
    child1->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    child1->SetClipContents(enableClip);
    child1->SetName("child1");
    parent1->AddControl(child1);
    AddControl(parent1);

    parentX = screenSize.dx - parentX - parentW;
    parentY += parentH * 0.5f;
    parent2 = new UIControl(Rect(parentX, parentY, parentW, parentH));
    parent2->SetDebugDraw(enableDebugDraw);
    parent2->GetBackground()->SetColor(Color(0.1f, 0.f, 0.5f, 1.f));
    parent2->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    parent2->SetClipContents(enableClip);
    parent2->SetName("parent2");
    child2 = new UIControl(Rect(-1.f * parentW * 0.5f, -1.f * parentH * 0.5f, parentW, parentH));
    child2->SetDebugDraw(enableDebugDraw);
    child2->GetBackground()->SetColor(Color(0.5f, 0.f, 0.1f, 1.f));
    child2->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    child2->SetClipContents(enableClip);
    child2->SetName("child2");
    parent2->AddControl(child2);
    AddControl(parent2);
    // button Clip
    clip = new UIButton(Rect(startX, startY, w, h));
    clip->SetStateFont(0xFF, font);
    clip->SetDebugDraw(enableClip);
    clip->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    clip->SetStateText(0xFF, L"clip");
    clip->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ClipTest::ClipPressed));
    AddControl(clip);
    startY += h + h * 0.5f;
    // button DebugDraw
    debugDraw = new UIButton(Rect(startX, startY, w, h));
    debugDraw->SetStateFont(0xFF, font);
    debugDraw->SetDebugDraw(enableDebugDraw);
    debugDraw->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    debugDraw->SetStateText(0xFF, L"debugDraw");
    debugDraw->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ClipTest::DebugDrawPressed));
    AddControl(debugDraw);
    startY += h + h * 0.5f;
    // button startPos
    startPos = new UIButton(Rect(startX, startY, w, h));
    startPos->SetStateFont(0xFF, font);
    startPos->SetDebugDraw(true);
    startPos->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    startPos->SetStateText(0xFF, L"startPos");
    startPos->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ClipTest::StartPos));
    AddControl(startPos);
    startY += h + h * 0.5f;
    // button moveLeft
    moveLeft = new UIButton(Rect(startX, startY, w, h));
    moveLeft->SetStateFont(0xFF, font);
    moveLeft->SetDebugDraw(true);
    moveLeft->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    moveLeft->SetStateText(0xFF, L"moveLeft");
    moveLeft->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ClipTest::MoveLeft));
    AddControl(moveLeft);
    startY += h + h * 0.5f;
    // button moveRight
    moveRight = new UIButton(Rect(startX, startY, w, h));
    moveRight->SetStateFont(0xFF, font);
    moveRight->SetDebugDraw(true);
    moveRight->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    moveRight->SetStateText(0xFF, L"moveRight");
    moveRight->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ClipTest::MoveRight));
    AddControl(moveRight);
    startY += h + h * 0.5f;
    // button moveUp
    moveUp = new UIButton(Rect(startX, startY, w, h));
    moveUp->SetStateFont(0xFF, font);
    moveUp->SetDebugDraw(true);
    moveUp->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    moveUp->SetStateText(0xFF, L"moveUp");
    moveUp->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ClipTest::MoveUp));
    AddControl(moveUp);
    startY += h + h * 0.5f;
    // button moveDown
    moveDown = new UIButton(Rect(startX, startY, w, h));
    moveDown->SetStateFont(0xFF, font);
    moveDown->SetDebugDraw(true);
    moveDown->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    moveDown->SetStateText(0xFF, L"moveDown");
    moveDown->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ClipTest::MoveDown));
    AddControl(moveDown);

    SafeRelease(font);
    BaseScreen::LoadResources();
}

void ClipTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    RemoveAllControls();
    SafeRelease(fullSizeWgt);
    SafeRelease(parent1);
    SafeRelease(child1);
    SafeRelease(parent2);
    SafeRelease(child2);

    SafeRelease(clip);
    SafeRelease(debugDraw);
    SafeRelease(startPos);
    SafeRelease(moveLeft);
    SafeRelease(moveRight);
    SafeRelease(moveUp);
    SafeRelease(moveDown);
    SafeRelease(fullSizeWgt);
}

