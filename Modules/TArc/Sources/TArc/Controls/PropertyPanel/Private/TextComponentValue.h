#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"

#include <QString>

namespace DAVA
{
namespace TArc
{
class TextComponentValue : public BaseComponentValue
{
public:
    QQmlComponent* GetComponent(QQmlEngine* engine) const override;

private:
    QString GetObjectName() const;

    QString GetText() const;
    void SetText(const QString& text);

    bool IsReadOnly() const;
    bool IsEnabled() const;

private:
    DAVA_VIRTUAL_REFLECTION(TextComponentValue, BaseComponentValue);
};
}
}