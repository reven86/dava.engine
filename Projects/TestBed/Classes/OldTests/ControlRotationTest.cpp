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


#include "ControlRotationTest.h"
#include "TextureUtils.h"


ControlRotationTest::ControlRotationTest()
: TestTemplate<ControlRotationTest>("ControlRotationTest")
{
    angle = 0.f;
    
    for(int32 i = 0; i < 100; ++i)
    {
        RegisterFunction(this, &ControlRotationTest::TestFunction, Format("Rotation_%d", i), NULL);
    }

}

void ControlRotationTest::LoadResources()
{
    GetBackground()->SetColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
    GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);

    rotatedControl = new UIControl(Rect(256.f, 256.f, 256.f, 256.f));
    rotatedControl->SetSprite("~res:/Gfx/UI/Rotation", 0);
    rotatedControl->pivotPoint = rotatedControl->GetSize()/2;

    rotatedControl->SetAngle(angle);
    AddControl(rotatedControl);
    
    
    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
    
    font->SetSize(20);
    
    description = new UIStaticText(Rect(0, 256, 512, 200));
    description->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
    description->SetMultiline(true);
    description->SetFont(font);
    AddControl(description);
    
    SafeRelease(font);
}


void ControlRotationTest::UnloadResources()
{
    RemoveAllControls();

    SafeRelease(rotatedControl);
}

void ControlRotationTest::TestFunction(PerfFuncData * data)
{
    angle += (PI / 4);
    rotatedControl->SetAngle(angle);
    
    description->SetText(Format(L"Angle %f", angle));

    int64 a = 0;
    for (int32 i = 0; i < 100; ++i)
    {
        for (int32 j = 0; j < 1000000; ++j)
        {
            a += i+j;
        }
    }
}




