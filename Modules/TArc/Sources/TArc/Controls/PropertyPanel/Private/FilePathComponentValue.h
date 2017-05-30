#pragma once
#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "TArc/WindowSubSystem/UI.h"

#include <Reflection/Reflection.h>
#include <Base/FastName.h>

namespace DAVA
{
namespace TArc
{
class FilePathComponentValue : public BaseComponentValue
{
public:
    FilePathComponentValue() = default;

protected:
    Any GetMultipleValue() const override;
    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override;
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) override;

private:
    Any GetFilePath() const;
    void SetFilePath(const Any& v);

    DAVA_VIRTUAL_REFLECTION(FilePathComponentValue, BaseComponentValue);
};
} // namespace TArc
} // namespace DAVA