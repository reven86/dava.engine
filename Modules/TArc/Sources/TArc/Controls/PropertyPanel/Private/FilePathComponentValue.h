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
    struct Params
    {
        String dialogTitle = "Open File";
        String filters = "";
        FilePath rootDir = "";
        UI* ui = nullptr;
        WindowKey wndKey = WindowKey(FastName(""));
    };
    FilePathComponentValue(const Params& params);

protected:
    Any GetMultipleValue() const override;
    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override;
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) const override;

private:
    Any GetFilePath() const;
    void SetFilePath(const Any& v);

    const String& GetDialogTitle() const;
    const String& GetFilters() const;
    const FilePath& GetRootDir() const;

    Params params;

    DAVA_VIRTUAL_REFLECTION(FilePathComponentValue, BaseComponentValue);
};
} // namespace TArc
} // namespace DAVA