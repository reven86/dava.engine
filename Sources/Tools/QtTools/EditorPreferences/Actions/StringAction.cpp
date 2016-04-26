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
#include "QtTools/EditorPreferences/Actions/StringAction.h"
#include "QtTools/EditorPreferences/PreferencesStorage.h"
#include <QInputDialog>
#include <QApplication>

StringAction::StringAction(const DAVA::InspMember* member_, QObject* parent)
    : AbstractAction(member_, parent)
{
}

void StringAction::OnTriggered(bool /*triggered*/)
{
    QString currentValue = data().toString();
    QString newVal = QInputDialog::getText(qApp->activeWindow(), QString("enter new value"), QString("enter new value of %s").arg(QString(member->Desc().text)), QLineEdit::Normal, currentValue);
    DAVA::VariantType newValueVar;
    switch (type)
    {
    case DAVA::VariantType::TYPE_STRING:
        newValueVar.SetString(newVal.toStdString());
        break;
    case DAVA::VariantType::TYPE_WIDE_STRING:
        newValueVar.SetWideString(newVal.toStdWString());
        break;
    case DAVA::VariantType::TYPE_FASTNAME:
        newValueVar.SetFastName(DAVA::FastName(newVal.toStdString()));
        break;
    case DAVA::VariantType::TYPE_FILEPATH:
        newValueVar.SetFilePath(DAVA::FilePath(newVal.toStdString()));
        break;
    default:
        DVASSERT(false && "bad type passed to factory");
        return;
    }
    PreferencesStorage::Instance()->SetValue(member, newValueVar);
}

void StringAction::OnValueChanged(const DAVA::VariantType& value)
{
    QString newVal;
    switch (value.type)
    {
    case DAVA::VariantType::TYPE_STRING:
        newVal = QString::fromStdString(value.AsString());
        break;
    case DAVA::VariantType::TYPE_WIDE_STRING:
        newVal = QString::fromStdWString(value.AsWideString());
        break;
    case DAVA::VariantType::TYPE_FASTNAME:
        newVal = QString(value.AsFastName().c_str());
        break;
    case DAVA::VariantType::TYPE_FILEPATH:
        newVal = QString::fromStdString(value.AsFilePath().GetStringValue());
        break;
    default:
        DVASSERT(false && "unknown type passed to IntAction");
        return;
    }
    setText(member->Desc().text + QString(" : %1").arg(newVal));
    setData(QVariant(newVal));
}
