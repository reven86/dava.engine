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
#include <QAction>
#include <QInputDialog>
#include <QApplication>
#include <QSet>
#include <QColorDialog>

namespace PreferencesFactory_local
{
class AbstractAction;

class ActionsStorage : public DAVA::TrackedObject
{
public:
    ActionsStorage();

    void OnPreferencesValueChanged(const DAVA::InspInfo* info, const DAVA::InspMember* member, const DAVA::VariantType&);

    void RegisterAction(const DAVA::InspInfo* info, const DAVA::InspMember* member, AbstractAction* action);
    void UnregisterAction(const DAVA::InspInfo* info, const DAVA::InspMember* member, AbstractAction* action);

private:
    QMap<const DAVA::InspInfo*, QMap<const DAVA::InspMember*, QSet<AbstractAction*>>> registeredActions;
};

ActionsStorage storage; //trollface

class AbstractAction : public QAction
{
public:
    AbstractAction(const DAVA::InspInfo* info_, const DAVA::InspMember* member_, QObject* parent)
        : QAction(parent)
        , info(info_)
        , member(member_)
    {
    }
    ~AbstractAction() override
    {
        storage.UnregisterAction(info, member, this);
    }
    virtual void OnValueChanged(const DAVA::VariantType& value) = 0;
    virtual void Init()
    {
        setText(member->Desc().text);
        storage.RegisterAction(info, member, this);
        DAVA::VariantType defaultValue = PreferencesStorage::GetPreferencesValue(member);
        OnValueChanged(defaultValue);
        type = defaultValue.type;

        connect(this, &QAction::triggered, this, &AbstractAction::OnTriggered);
    }

protected:
    virtual void OnTriggered(bool triggered) = 0;

    const DAVA::InspInfo* info = nullptr;
    const DAVA::InspMember* member = nullptr;
    DAVA::uint8 type = DAVA::VariantType::TYPE_NONE;
};

ActionsStorage::ActionsStorage()
{
    PreferencesStorage::Instance()->ValueChanged.Connect(this, &ActionsStorage::OnPreferencesValueChanged);
}

void ActionsStorage::OnPreferencesValueChanged(const DAVA::InspInfo* info, const DAVA::InspMember* member, const DAVA::VariantType& value)
{
    QSet<AbstractAction*> actions = registeredActions[info][member];
    for (auto action : actions)
    {
        action->OnValueChanged(value);
    }
}

void ActionsStorage::RegisterAction(const DAVA::InspInfo* info, const DAVA::InspMember* member, AbstractAction* action)
{
    registeredActions[info][member].insert(action);
}

void ActionsStorage::UnregisterAction(const DAVA::InspInfo* info, const DAVA::InspMember* member, AbstractAction* action)
{
    registeredActions[info][member].remove(action);
}

class BoolAction : public AbstractAction
{
public:
    BoolAction(const DAVA::InspInfo* info_, const DAVA::InspMember* member_, QObject* parent)
        : AbstractAction(info_, member_, parent)
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
        PreferencesStorage::SetNewValueToAllRegisteredObjects(info, member, DAVA::VariantType(triggered));
    }
};

class IntAction : public AbstractAction
{
public:
    IntAction(const DAVA::InspInfo* info_, const DAVA::InspMember* member_, QObject* parent)
        : AbstractAction(info_, member_, parent)
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
        PreferencesStorage::SetNewValueToAllRegisteredObjects(info, member, newValueVar);
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
    DoubleAction(const DAVA::InspInfo* info_, const DAVA::InspMember* member_, QObject* parent)
        : AbstractAction(info_, member_, parent)
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
            newValueVar.SetFloat(newVal);
            break;
        case DAVA::VariantType::TYPE_FLOAT64:
            newValueVar.SetFloat64(newVal);
            break;
        default:
            DVASSERT(false && "bad type passed to factory");
            return;
        }
        PreferencesStorage::SetNewValueToAllRegisteredObjects(info, member, newValueVar);
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
    StringAction(const DAVA::InspInfo* info_, const DAVA::InspMember* member_, QObject* parent)
        : AbstractAction(info_, member_, parent)
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
            newValueVar.SetFastName(DAVA::FastName(newVal.toStdString().c_str()));
            break;
        case DAVA::VariantType::TYPE_FILEPATH:
            newValueVar.SetFilePath(DAVA::FilePath(newVal.toStdString()));
            break;
        default:
            DVASSERT(false && "bad type passed to factory");
            return;
        }
        PreferencesStorage::SetNewValueToAllRegisteredObjects(info, member, newValueVar);
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
            newVal = QString::fromStdString(value.AsFilePath().GetAbsolutePathname());
            break;
        default:
            DVASSERT(false && "unknown type passed to IntAction");
            return;
        }
        setText(member->Desc().text + QString(" : %1").arg(newVal));
        setData(QVariant(newVal));
    }
};

DAVA::Color QColorToColor(const QColor& qtColor)
{
    return DAVA::Color(qtColor.redF(), qtColor.greenF(), qtColor.blueF(), qtColor.alphaF());
}

QColor ColorToQColor(const DAVA::Color& davaColor)
{
    return QColor((int)DAVA::Round(davaColor.r * 255.0f), (int)DAVA::Round(davaColor.g * 255.0f), (int)DAVA::Round(davaColor.b * 255.0f), (int)DAVA::Round(davaColor.a * 255.0f));
}

class ColorAction : public AbstractAction
{
public:
    ColorAction(const DAVA::InspInfo* info_, const DAVA::InspMember* member_, QObject* parent)
        : AbstractAction(info_, member_, parent)
    {
    }

    void OnTriggered(bool /*triggered*/) override
    {
        QColor currentValue = data().value<QColor>();
        QColor color = QColorDialog::getColor(currentValue, qApp->activeWindow(), "Select color", QColorDialog::ShowAlphaChannel);
        if (!color.isValid())
        {
            return;
        }
        DAVA::VariantType newValueVar;
        switch (type)
        {
        case DAVA::VariantType::TYPE_COLOR:
            newValueVar.SetColor(QColorToColor(color));
            break;
        default:
            DVASSERT(false && "bad type passed to factory");
            return;
        }
        PreferencesStorage::SetNewValueToAllRegisteredObjects(info, member, newValueVar);
    }

    void OnValueChanged(const DAVA::VariantType& value) override
    {
        QColor color;
        switch (value.type)
        {
        case DAVA::VariantType::TYPE_COLOR:
            color = ColorToQColor(value.AsColor());
            break;
        default:
            DVASSERT(false && "unknown type passed to IntAction");
            return;
        }
        QPixmap pixmap(16, 16);
        pixmap.fill(color);
        setIcon(pixmap);
        setData(color);
    }
};
};

QAction* PreferencesActionsFactory::CreateActionForPreference(const DAVA::FastName& className, const DAVA::FastName& propertyName, QObject* parent)
{
    const DAVA::InspInfo* inspInfo = PreferencesStorage::GetInspInfo(className);
    const DAVA::InspMember* inspMember = PreferencesStorage::GetInspMember(inspInfo, propertyName);
    const DAVA::MetaInfo* metaInfo = inspMember->Type();
    DVASSERT(inspInfo != nullptr && inspMember != nullptr);
    PreferencesFactory_local::AbstractAction* action = nullptr;
    if (metaInfo == DAVA::MetaInfo::Instance<bool>())
    {
        action = new PreferencesFactory_local::BoolAction(inspInfo, inspMember, parent);
    }
    if (metaInfo == DAVA::MetaInfo::Instance<DAVA::int32>()
        || metaInfo == DAVA::MetaInfo::Instance<DAVA::int64>()
        || metaInfo == DAVA::MetaInfo::Instance<DAVA::uint32>()
        || metaInfo == DAVA::MetaInfo::Instance<DAVA::uint64>())
    {
        action = new PreferencesFactory_local::IntAction(inspInfo, inspMember, parent);
    }
    if (metaInfo == DAVA::MetaInfo::Instance<DAVA::float32>()
        || metaInfo == DAVA::MetaInfo::Instance<DAVA::float64>())
    {
        action = new PreferencesFactory_local::DoubleAction(inspInfo, inspMember, parent);
    }
    if (metaInfo == DAVA::MetaInfo::Instance<DAVA::String>()
        || metaInfo == DAVA::MetaInfo::Instance<DAVA::WideString>()
        || metaInfo == DAVA::MetaInfo::Instance<DAVA::FastName>()
        || metaInfo == DAVA::MetaInfo::Instance<DAVA::FilePath>())
    {
        action = new PreferencesFactory_local::StringAction(inspInfo, inspMember, parent);
    }
    if (metaInfo == DAVA::MetaInfo::Instance<DAVA::Color>())
    {
        action = new PreferencesFactory_local::ColorAction(inspInfo, inspMember, parent);
    }
    DVASSERT(nullptr != action && "please create delegate for type");
    action->Init();
    return action;
}
