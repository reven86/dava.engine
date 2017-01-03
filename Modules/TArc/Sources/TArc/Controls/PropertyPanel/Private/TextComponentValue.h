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

protected:
    virtual Any Convert(const DAVA::String& text) const;

private:
    QString GetObjectName() const;

    DAVA::String GetText() const;
    void SetText(const DAVA::String& text);

    bool IsReadOnly() const;
    bool IsEnabled() const;

private:
    DAVA_VIRTUAL_REFLECTION(TextComponentValue, BaseComponentValue);
};

class FastNameComponentValue : public TextComponentValue
{
private:
    Any Convert(const DAVA::String& text) const;

    DAVA_VIRTUAL_REFLECTION(FastNameComponentValue, TextComponentValue);
};
}
}