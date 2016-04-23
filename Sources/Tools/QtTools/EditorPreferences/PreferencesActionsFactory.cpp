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


#include "Base/FastName.h"
#include "Base/Introspection.h"
#include "Functional/Signal.h"
#include "QtTools/EditorPreferences/PreferencesActionsFactory.h"
#include "QtTools/EditorPreferences/PreferencesStorage.h"
#include "QtTools/Utils/Utils.h"

#include <QAction>
#include <QInputDialog>
#include <QApplication>
#include <QSet>
#include <QColorDialog>
#include <QPainter>

namespace PreferencesFactory_local
{
class AbstractAction;

class ActionsStorage : public DAVA::TrackedObject
{
public:
    ActionsStorage();

    void OnPreferencesValueChanged(const DAVA::InspMember* member, const DAVA::VariantType& value);

    void RegisterAction(AbstractAction* action);
    void UnregisterAction(AbstractAction* action);

private:
    DAVA::UnorderedMap<const DAVA::InspMember*, DAVA::Set<AbstractAction*>> registeredActions;
};

ActionsStorage storage;

class AbstractAction : public QAction
{
    friend class ActionsStorage;

public:
    AbstractAction(const DAVA::InspMember* member_, QObject* parent)
        : QAction(parent)
        , member(member_)
    {
        storage.RegisterAction(this);
        setText(member->Desc().text);
    }
    ~AbstractAction() override
    {
        storage.UnregisterAction(this);
    }
    virtual void OnValueChanged(const DAVA::VariantType& value) = 0;
    virtual void Init()
    {
        DAVA::VariantType defaultValue = PreferencesStorage::Instance()->GetValue(member);
        OnValueChanged(defaultValue);
        type = defaultValue.type;

        connect(this, &QAction::triggered, this, &AbstractAction::OnTriggered);
    }

protected:
    virtual void OnTriggered(bool triggered) = 0;

    const DAVA::InspMember* member = nullptr;
    DAVA::uint8 type = DAVA::VariantType::TYPE_NONE;
};

ActionsStorage::ActionsStorage()
{
    PreferencesStorage::Instance()->valueChanged.Connect(this, &ActionsStorage::OnPreferencesValueChanged);
}

void ActionsStorage::OnPreferencesValueChanged(const DAVA::InspMember* member, const DAVA::VariantType& value)
{
    for (AbstractAction* action : registeredActions[member])
    {
        action->OnValueChanged(value);
    }
}

void ActionsStorage::RegisterAction(AbstractAction* action)
{
    registeredActions[action->member].insert(action);
}

void ActionsStorage::UnregisterAction(AbstractAction* action)
{
    DVVERIFY(registeredActions[action->member].erase(action) > 0);
}

class BoolAction : public AbstractAction
{
public:
    BoolAction(const DAVA::InspMember* member_, QObject* parent)
        : AbstractAction(member_, parent)
    {
    }

    void Init() override
    {
        AbstractAction::Init();
        setCheckable(true);
    }
    void OnValueChanged(const DAVA::VariantType& value) override
    {
        setChecked(value.AsBool());
    }

    void OnTriggered(bool triggered) override
    {
        PreferencesStorage::Instance()->SetValue(member, DAVA::VariantType(triggered));
    }
};

class IntAction : public AbstractAction
{
public:
    IntAction(const DAVA::InspMember* member_, QObject* parent)
        : AbstractAction(member_, parent)
    {
    }

    void OnTriggered(bool /*triggered*/) override
    {
        int currentValue = data().toInt();
        int newVal = QInputDialog::getInt(qApp->activeWindow(), QString("enter new value"), QString("enter new value of %s").arg(QString(member->Desc().text)), currentValue);
        DAVA::VariantType newValueVar;
        switch (type)
        {
        case DAVA::VariantType::TYPE_INT32:
            newValueVar.SetInt32(newVal);
            break;
        case DAVA::VariantType::TYPE_INT64:
            newValueVar.SetInt64(newVal);
            break;
        case DAVA::VariantType::TYPE_UINT32:
            newValueVar.SetUInt32(newVal);
            break;
        case DAVA::VariantType::TYPE_UINT64:
            newValueVar.SetUInt64(newVal);
            break;
        default:
            DVASSERT(false && "bad type passed to factory");
            return;
        }
        PreferencesStorage::Instance()->SetValue(member, newValueVar);
    }
    void OnValueChanged(const DAVA::VariantType& value) override
    {
        int newVal = 0;
        switch (value.type)
        {
        case DAVA::VariantType::TYPE_INT32:
            newVal = value.AsInt32();
            break;
        case DAVA::VariantType::TYPE_INT64:
            newVal = value.AsInt64();
            break;
        case DAVA::VariantType::TYPE_UINT32:
            newVal = value.AsUInt32();
            break;
        case DAVA::VariantType::TYPE_UINT64:
            newVal = value.AsInt64();
            break;
        default:
            DVASSERT(false && "unknown type passed to IntAction");
            return;
        }
        setText(member->Desc().text + QString(" : %1").arg(newVal));
        setData(QVariant(newVal));
    }
};

class DoubleAction : public AbstractAction
{
public:
    DoubleAction(const DAVA::InspMember* member_, QObject* parent)
        : AbstractAction(member_, parent)
    {
    }

    void OnTriggered(bool /*triggered*/) override
    {
        double currentValue = data().toDouble();
        double newVal = QInputDialog::getDouble(qApp->activeWindow(), QString("enter new value"), QString("enter new value of %s").arg(QString(member->Desc().text)), currentValue);
        DAVA::VariantType newValueVar;
        switch (type)
        {
        case DAVA::VariantType::TYPE_FLOAT:
            newValueVar.SetFloat(static_cast<DAVA::float32>(newVal));
            break;
        case DAVA::VariantType::TYPE_FLOAT64:
            newValueVar.SetFloat64(newVal);
            break;
        default:
            DVASSERT(false && "bad type passed to factory");
            return;
        }
        PreferencesStorage::Instance()->SetValue(member, newValueVar);
    }

    void OnValueChanged(const DAVA::VariantType& value) override
    {
        double newVal = 0.0f;
        switch (value.type)
        {
        case DAVA::VariantType::TYPE_FLOAT:
            newVal = value.AsFloat();
            break;
        case DAVA::VariantType::TYPE_FLOAT64:
            newVal = value.AsFloat64();
            break;
        default:
            DVASSERT(false && "unknown type passed to IntAction");
            return;
        }
        setText(member->Desc().text + QString(" : %1").arg(newVal));
        setData(QVariant(newVal));
    }
};

class StringAction : public AbstractAction
{
public:
    StringAction(const DAVA::InspMember* member_, QObject* parent)
        : AbstractAction(member_, parent)
    {
    }

    void OnTriggered(bool /*triggered*/) override
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

    void OnValueChanged(const DAVA::VariantType& value) override
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
};

class ColorAction : public AbstractAction
{
public:
    ColorAction(const DAVA::InspMember* member_, QObject* parent)
        : AbstractAction(member_, parent)
    {
    }

    void OnTriggered(bool /*triggered*/) override
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

    void OnValueChanged(const DAVA::VariantType& value) override
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
};
};

QAction* PreferencesActionsFactory::CreateActionForPreference(const DAVA::FastName& className, const DAVA::FastName& propertyName, QObject* parent)
{
    const DAVA::InspInfo* inspInfo = PreferencesStorage::Instance()->GetInspInfo(className);
    const DAVA::InspMember* inspMember = inspInfo->Member(propertyName);
    DVASSERT(inspInfo != nullptr && inspMember != nullptr);
    const DAVA::MetaInfo* metaInfo = inspMember->Type();
    PreferencesFactory_local::AbstractAction* action = nullptr;
    if (metaInfo == DAVA::MetaInfo::Instance<bool>())
    {
        action = new PreferencesFactory_local::BoolAction(inspMember, parent);
    }
    else if (metaInfo == DAVA::MetaInfo::Instance<DAVA::int32>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::int64>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::uint32>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::uint64>())
    {
        action = new PreferencesFactory_local::IntAction(inspMember, parent);
    }
    else if (metaInfo == DAVA::MetaInfo::Instance<DAVA::float32>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::float64>())
    {
        action = new PreferencesFactory_local::DoubleAction(inspMember, parent);
    }
    else if (metaInfo == DAVA::MetaInfo::Instance<DAVA::String>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::WideString>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::FastName>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::FilePath>())
    {
        action = new PreferencesFactory_local::StringAction(inspMember, parent);
    }
    else if (metaInfo == DAVA::MetaInfo::Instance<DAVA::Color>())
    {
        action = new PreferencesFactory_local::ColorAction(inspMember, parent);
    }
    else
    {
        DVASSERT(nullptr != action && "please create delegate for type");
        return nullptr;
    }
    action->Init();
    return action;
}
