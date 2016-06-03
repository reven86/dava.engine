#pragma once

#include "FileSystem/VariantType.h"

#include <QAction>

namespace DAVA
{
class InspMember;
}

class AbstractAction : public QAction
{
    friend class ActionsStorage;

public:
    AbstractAction(const DAVA::InspMember* member, QObject* parent);
    ~AbstractAction() override;
    virtual void OnValueChanged(const DAVA::VariantType& value) = 0;
    virtual void Init();

protected:
    virtual void OnTriggered(bool triggered) = 0;

    const DAVA::InspMember* member = nullptr;
    DAVA::uint8 type = DAVA::VariantType::TYPE_NONE;
};
