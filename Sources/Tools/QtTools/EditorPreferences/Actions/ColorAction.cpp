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


#include "Base/Introspection.h"
#include "QtTools/EditorPreferences/Actions/ColorAction.h"
#include "QtTools/EditorPreferences/PreferencesStorage.h"
#include "QtTools/Utils/Utils.h"
#include <QColorDialog>
#include <QApplication>

ColorAction::ColorAction(const DAVA::InspMember* member_, QObject* parent)
    : AbstractAction(member_, parent)
{
}

void ColorAction::OnTriggered(bool /*triggered*/)
{
    if (type != DAVA::VariantType::TYPE_COLOR)
    {
        DVASSERT(false && "bad type passed to factory");
        return;
    }
    QColor currentValue = data().value<QColor>();

    QColor color = QColorDialog::getColor(currentValue, qApp->activeWindow(), "Select color", QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
    if (!color.isValid())
    {
        return;
    }
    DAVA::VariantType newValueVar(QColorToColor(color));
    PreferencesStorage::Instance()->SetValue(member, newValueVar);
}

void ColorAction::OnValueChanged(const DAVA::VariantType& value)
{
    if (value.type != DAVA::VariantType::TYPE_COLOR)
    {
        DVASSERT(false && "unknown type passed to IntAction");
        return;
    }

    QColor color = ColorToQColor(value.AsColor());
    setIcon(CreateIconFromColor(color));
    setData(color);
}
