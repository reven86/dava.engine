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


#include "AdjustablePointDoubleSpinbox.h"
#include "Base/BaseMath.h"
#include "Math/MathHelpers.h"

AdjustablePointDoubleSpinBox::AdjustablePointDoubleSpinBox(QWidget* parent) :
    QDoubleSpinBox(parent)
{
}

void AdjustablePointDoubleSpinBox::SetValueAndAdjustPoint(double value)
{
    double fracPart = fmodf(value, 1.0f);

    static const double roundingMultiplier = 1000.0f; // 4 digits.
    static const int minDecimalPlaces = 1;

    double rounded = DAVA::Round(fracPart * roundingMultiplier) / roundingMultiplier;

    char buf[8] = {0};
    sprintf(buf, "%.4g", rounded);

    int curDecimalPlaces = strlen(buf) - 2; // 2 means size of leading "0."

    setDecimals(qMax(curDecimalPlaces, minDecimalPlaces));
    setValue(value);
}
