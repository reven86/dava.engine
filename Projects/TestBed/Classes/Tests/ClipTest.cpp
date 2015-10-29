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
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"

using namespace ::Windows::System;
using namespace ::Windows::Foundation;
using namespace ::Windows::UI::Core;
using namespace ::Windows::UI::Xaml;
using namespace ::Windows::UI::Xaml::Input;
using namespace ::Windows::UI::Xaml::Controls;
using namespace ::Windows::UI::Input;
using namespace ::Windows::UI::ViewManagement;
using namespace ::Windows::Devices::Input;
using namespace ::Windows::ApplicationModel;
using namespace ::Windows::Graphics::Display;
using namespace ::Windows::ApplicationModel::Core;
using namespace ::Windows::UI::Xaml::Media;
using namespace ::Windows::System::Threading;

using namespace DAVA;

ClipTest::ClipTest ()
    : BaseScreen("ClipTest")
{
}

void ClipTest::PreSet()
{
    //pass notification in main thread
    DAVA::CorePlatformWinUAP* core = static_cast<DAVA::CorePlatformWinUAP*>(DAVA::Core::Instance());

    core->RunOnUIThreadBlocked([=] 
    { 
        ApplicationView::GetForCurrentView()->PreferredLaunchViewSize = Windows::Foundation::Size(IOS_WIDTH, IOS_HEIGHT);
        ApplicationView::PreferredLaunchWindowingMode = ApplicationViewWindowingMode::PreferredLaunchViewSize;
    });

    Sleep(100);

    //from Client's FrameworkDidLaunched
    int32 virtualWidth = IOS_WIDTH;
    int32 virtualHeight = IOS_HEIGHT;

    DAVA::VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(virtualWidth, virtualHeight);
    DAVA::VirtualCoordinatesSystem::Instance()->SetProportionsIsFixed(false);
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(virtualWidth, virtualHeight, "Gfx");
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(virtualWidth * 2, virtualHeight * 2, "Gfx2");
}

void ClipTest::LoadResources()
{
    static bool isInit = false;
    if (!isInit)
    {
        isInit = false;
        PreSet();
    }

//     UIYamlLoader::Load(this, "~res:/UI/Test/ClipTest.yaml");
//     parent = DynamicTypeCheck<UIScrollView *>(FindByName("Settings"));
//     parent = DynamicTypeCheck<UIControl *>(FindByName("background"));
//     child = DynamicTypeCheck<UIControl *>(FindByName("fullscreenContent"));
    bool clip_flg = true;
    bool debug_flg = false;
    float32 shift_x = 0;
    float32 shift_y = 0;


    UIControl *parent = new UIControl(DAVA::Rect(0.f, 0.f, float32(IOS_WIDTH), float32(IOS_HEIGHT)));
    parent->SetDebugDraw(debug_flg);
    parent->GetBackground()->SetColor(Color(1.f, 1.f, 1.f, 1.f));
    parent->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    parent->SetClipContents(clip_flg);
    parent->SetName("parent");

    // test 1 
    UIControl *test1_1 = new UIControl(DAVA::Rect(shift_x + 0.f, float32(IOS_HEIGHT) - 400.f, float32(IOS_WIDTH), 400.f));
    test1_1->SetDebugDraw(debug_flg);
    test1_1->GetBackground()->SetColorInheritType(UIControlBackground::COLOR_IGNORE_PARENT);
    test1_1->GetBackground()->SetColor(Color(0.0, 1.0, 0.0, 1.0000));
    test1_1->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    test1_1->SetClipContents(clip_flg);
    test1_1->SetName("test1_1");
    UIControl *test1_2 = new UIControl(DAVA::Rect(0.f, float32(IOS_HEIGHT) - 200.f, float32(IOS_WIDTH), 200.f));
    test1_2->SetDebugDraw(debug_flg);
    test1_2->GetBackground()->SetColorInheritType(UIControlBackground::COLOR_IGNORE_PARENT);
    test1_2->GetBackground()->SetColor(Color(0.0, 0.0, 1.0, 1.0));
    test1_2->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    test1_2->SetClipContents(clip_flg);
    test1_2->SetName("test1_2");
//    test1_1->AddControl(test1_2);
    UIControl *test1_3 = new UIControl(DAVA::Rect(0.f, 100.f, float32(IOS_WIDTH), 100.f));
    test1_3->SetDebugDraw(debug_flg);
    test1_3->GetBackground()->SetColorInheritType(UIControlBackground::COLOR_IGNORE_PARENT);
    test1_3->GetBackground()->SetColor(Color(0.6431, 1.0, 0.0, 1.0));
    test1_3->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    test1_3->SetClipContents(clip_flg);
    test1_3->SetName("test1_3");
    test1_2->AddControl(test1_3);

// 
// 
//     UIControl *test2 = new UIControl(DAVA::Rect(0.f, float32(IOS_HEIGHT) - 200.f, float32(IOS_WIDTH), 200.f));
//     test2->SetDebugDraw(true);
//     test2->GetBackground()->SetColor(Color(0.6431, 0.0627, 1.0000, 1.0000));
//     test2->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
//     test2->SetClipContents(true);
// 
//     UIControl *test3 = new UIControl(DAVA::Rect(0.f, 180.f, float32(IOS_WIDTH), float32(IOS_HEIGHT) - 180.f));
//     test3->SetDebugDraw(true);
//     test3->GetBackground()->SetColor(Color(0.6431, 0.0627, 0.8000, 0.5000));
//     test3->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
// 
// 
//     test1->AddControl(test2);
// 
    
    parent->AddControl(test1_1);
    AddControl(parent);
    parent->Update(0);

    BaseScreen::LoadResources();
    //TODO: Initialize resources here
}

void ClipTest::UnloadResources()
{
    RemoveAllControls();

    BaseScreen::UnloadResources();
    //TODO: Release resources here
}

