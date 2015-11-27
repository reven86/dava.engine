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


#include "Tests/FloatingPointExceptionTest.h"

#include <numeric>

FloatingPointExceptionTest::FloatingPointExceptionTest ()
    : BaseScreen("FloatingPointExceptionTest")
{
}

void do_structured_exception(DAVA::float32 max_value)
{
    DAVA::float32 over_value = max_value / 0.f; // overflow
    DAVA::float32 inf_mul_zero = over_value * 0.f;
    DAVA::float32 nan_div_nan = inf_mul_zero / inf_mul_zero;
    DAVA::Logger::Debug("sum of float max == %f(max_value == %f), inf_mul_zero == %f, nan_div_nan == %f", over_value, max_value, inf_mul_zero, nan_div_nan);
}

void DoFloatingPointException(DAVA::BaseObject*, void*, void*)
{
    // TODO
    DAVA::Logger::Debug("try overflow floating point number");
    DAVA::float32 max_value = std::numeric_limits<float>::max();
    try
    {
        do_structured_exception(2.f);
        DAVA::Logger::Debug("no exception on float overflow! max float + max float");
    } catch (std::exception& ex)
    {
        DAVA::Logger::Debug("catch floating point exception: %s", ex.what());
    }
}

void FloatingPointExceptionTest::LoadResources()
{
    BaseScreen::LoadResources();
    
    DAVA::ScopedPtr<DAVA::FTFont> font(DAVA::FTFont::Create("~res:/Fonts/korinna.ttf"));

    DAVA::UIButton* resetButton = new DAVA::UIButton(DAVA::Rect(420, 30, 200, 30));
    resetButton->SetDebugDraw(true);
    resetButton->SetStateFont(0xFF, font);
    resetButton->SetStateFontColor(0xFF, DAVA::Color::White);
    resetButton->SetStateText(0xFF, L"Generate Floating point exception");
    resetButton->AddEvent(DAVA::UIButton::EVENT_TOUCH_DOWN, DAVA::Message(&DoFloatingPointException));
    AddControl(resetButton);

}

void FloatingPointExceptionTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    //TODO: Release resources here
}

