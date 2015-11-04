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


#include "Infrastructure/TestListScreen.h"
#include <Utils/UTF8Utils.h>

using namespace DAVA;

TestListScreen::TestListScreen()
    : UIScreen()
    , testsGrid(nullptr)
    , testsGrid2(nullptr)
    , cellHeight(50)
{
}

TestListScreen::~TestListScreen()
{
    for(auto screen : testScreens)
    {
        SafeRelease(screen);
    }
    testScreens.clear();
}

void TestListScreen::AddTestScreen(BaseScreen *screen)
{
    screen->Retain();
    testScreens.push_back(screen);
}

void TestListScreen::LoadResources()
{
    UIScreen::LoadResources();
    
    Size2i screenSize = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize();
    
    testsGrid = new UIList(Rect(0.0, 0.0, static_cast<DAVA::float32>(screenSize.dx/2), static_cast<DAVA::float32>(screenSize.dy)), UIList::ORIENTATION_VERTICAL);
    testsGrid->SetDelegate(this);
    AddControl(testsGrid);
    
    testsGrid2 = new UIList(Rect(static_cast<DAVA::float32>(screenSize.dx/2)+1, 0.0, static_cast<DAVA::float32>(screenSize.dx/2)-1, static_cast<DAVA::float32>(screenSize.dy)), UIList::ORIENTATION_VERTICAL);
    testsGrid2->SetDelegate(this);
    AddControl(testsGrid2);
}

void TestListScreen::UnloadResources()
{
    UIScreen::UnloadResources();
    RemoveAllControls();
    
    SafeRelease(testsGrid);
    SafeRelease(testsGrid2);
}

int32 TestListScreen::ElementsCount(UIList * list)
{
    return static_cast<int32>(testScreens.size());
}

float32 TestListScreen::CellHeight(UIList * list, int32 index)
{
    return cellHeight;
}

UIListCell *TestListScreen::CellAtIndex(UIList *list, int32 index)
{
    const char8 buttonName[] = "CellButton";
    const char8 cellName[] = "TestButtonCell";
    UIStaticText *buttonText = nullptr;
    UIListCell *c = list->GetReusableCell(cellName); //try to get cell from the reusable cells store
    if(!c)
    { //if cell of requested type isn't find in the store create new cell
        c = new UIListCell(Rect(0., 0., static_cast<float32>(list->size.x), CellHeight(list, index)), cellName);
        
        buttonText = new UIStaticText(Rect(0., 0., static_cast<float32>(list->size.x), CellHeight(list, index)));
        buttonText->SetName(buttonName);
        c->AddControl(buttonText);
        
        Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
        DVASSERT(font);

        font->SetSize(static_cast<float32>(20));
        buttonText->SetFont(font);
        buttonText->SetDebugDraw(true);

        SafeRelease(font);
        c->GetBackground()->SetColor(Color(0.75, 0.75, 0.75, 0.5));
    }
    
    auto screen = testScreens.at(index);

    buttonText = (UIStaticText *)c->FindByName(buttonName);
    if (nullptr != buttonText)
    {
        buttonText->SetText(UTF8Utils::EncodeToWideString(screen->GetName()));
    }
    
    return c;//returns cell
}

void TestListScreen::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
    UIScreenManager::Instance()->SetScreen(selectedCell->GetIndex()+1);
}
