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


#include "Tests/PackManagerTest.h"
#include <UI/Focus/UIFocusComponent.h>
#include <PackManager/PackManager.h>

using namespace DAVA;

PackManagerTest::PackManagerTest()
    : BaseScreen("PackManagerTest")
{
}

void PackManagerTest::LoadResources()
{
    BaseScreen::LoadResources();

    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    packInput = new UITextField(Rect(5, 10, 400, 20));
    packInput->SetFont(font);
    packInput->SetText(L"unit_test.pak");
    packInput->SetDebugDraw(true);
    packInput->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    packInput->SetInputEnabled(true);
    packInput->GetOrCreateComponent<UIFocusComponent>();
    packInput->SetDelegate(new UITextFieldDelegate());
    packInput->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    AddControl(packInput);

    startLoadingButton = new UIButton(Rect(420, 10, 100, 20));
    startLoadingButton->SetDebugDraw(true);
    startLoadingButton->SetStateFont(0xFF, font);
    startLoadingButton->SetStateFontColor(0xFF, Color::White);
    startLoadingButton->SetStateText(0xFF, L"start loading");
    startLoadingButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &PackManagerTest::OnStartDownloadClicked));
    AddControl(startLoadingButton);

    packNameLoading = new UIStaticText(Rect(5, 300, 500, 20));
    packNameLoading->SetFont(font);
    packNameLoading->SetTextColor(Color::White);
    packNameLoading->SetMultiline(true);
    packNameLoading->SetText(L"loading: ");
    packNameLoading->SetDebugDraw(true);
    packNameLoading->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    AddControl(packNameLoading);

    redControl = new UIControl(Rect(5, 360, 500, 10));
    redControl->SetDebugDrawColor(Color(1.f, 0.f, 0.f, 1.f));
    redControl->SetDebugDraw(true);
    AddControl(redControl);

    greenControl = new UIControl(Rect(5, 360, 0, 10));
    greenControl->SetDebugDrawColor(Color(0.f, 1.f, 0.f, 1.f));
    greenControl->SetDebugDraw(true);
    AddControl(greenControl);

    description = new UIStaticText(Rect(5, 40, 400, 200));
    description->SetFont(font);
    description->SetTextColor(Color::White);
    description->SetMultiline(true);
    description->SetDebugDraw(true);
    description->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    String message = DAVA::Format("type name of pack you want to download\n"
                                  "Directory to downloaded packs: \"%s\"\nUrl to packs: \"%s\"\n"
                                  "When you press \"start loading\" full reinitializetion begins",
                                  folderWithDownloadedPacks.GetAbsolutePathname().c_str(), urlToServerWithPacks.c_str());
    description->SetText(UTF8Utils::EncodeToWideString(message));
    AddControl(description);
}

void PackManagerTest::UnloadResources()
{
    RemoveControl(packInput);
    SafeRelease(packInput);
    RemoveControl(startLoadingButton);
    SafeRelease(startLoadingButton);
    RemoveControl(packNameLoading);
    SafeRelease(packNameLoading);
    RemoveControl(redControl);
    SafeRelease(redControl);
    RemoveControl(greenControl);
    SafeRelease(greenControl);
    RemoveControl(description);
    SafeRelease(description);

    BaseScreen::UnloadResources();
}

void PackManagerTest::OnPackStateChange(const DAVA::PackManager::Pack& pack, DAVA::PackManager::Pack::Change change)
{
    if (change == PackManager::Pack::Change::DownloadProgress)
    {
        packNameLoading->SetText(UTF8Utils::EncodeToWideString("loading: " + pack.name));

        auto rect = redControl->GetRect();
        rect.dx = rect.dx * pack.downloadProgress;
        greenControl->SetRect(rect);
    }
    else if (change == PackManager::Pack::Change::State)
    {
        if (pack.state == PackManager::Pack::Mounted)
        {
            packNameLoading->SetText(UTF8Utils::EncodeToWideString("loading: " + pack.name + " done!"));
        }
    }
}

void PackManagerTest::OnStartDownloadClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    PackManager& packManager = Core::Instance()->GetPackManager();

    const Vector<PackManager::Pack>& packs = packManager.GetPacks();

    std::for_each(begin(packs), end(packs), [&packManager](const PackManager::Pack& pack)
                  {
                      if (pack.state == PackManager::Pack::Mounted)
                      {
                          packManager.Delete(pack.name);
                      }
                  });

    FileSystem::Instance()->DeleteDirectory(folderWithDownloadedPacks);
    FileSystem::Instance()->CreateDirectory(folderWithDownloadedPacks, true);

    // clear and renew all packs state
    packManager.Initialize(sqliteDbFile, folderWithDownloadedPacks, urlToServerWithPacks);
    packManager.EnableProcessing();

    packManager.onPackStateChanged.DisconnectAll();

    packManager.onPackStateChanged.Connect(this, &PackManagerTest::OnPackStateChange);

    String packName = UTF8Utils::EncodeToUTF8(packInput->GetText());

    try
    {
        packNameLoading->SetText(UTF8Utils::EncodeToWideString("loading: " + packName));
        packManager.RequestPack(packName);
    }
    catch (std::exception& ex)
    {
        packNameLoading->SetText(UTF8Utils::EncodeToWideString(ex.what()));
    }
}
