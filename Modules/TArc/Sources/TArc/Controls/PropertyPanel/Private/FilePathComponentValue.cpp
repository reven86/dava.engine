#include "TArc/Controls/PropertyPanel/Private/FilePathComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/ComponentStructureWrapper.h"
#include "TArc/Controls/PropertyPanel/PropertyPanelMeta.h"
#include "TArc/Controls/FilePathEdit.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
namespace TArc
{
Any FilePathComponentValue::GetMultipleValue() const
{
    return Any();
}

bool FilePathComponentValue::IsValidValueToSet(const Any& newValue, const Any& currentValue) const
{
    if (currentValue.CanCast<FilePath>() == false)
    {
        return true;
    }

    return newValue.Cast<FilePath>() != currentValue.Cast<FilePath>();
}

ControlProxy* FilePathComponentValue::CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) const
{
    FilePathEdit::Params p;
    p.ui = GetUI();
    p.wndKey = GetWindowKey();
    p.fields[FilePathEdit::Fields::Value] = "value";
    p.fields[FilePathEdit::Fields::IsReadOnly] = readOnlyFieldName;
    return new FilePathEdit(p, wrappersProcessor, model, parent);
}

Any FilePathComponentValue::GetFilePath() const
{
    return GetValue();
}

void FilePathComponentValue::SetFilePath(const Any& v)
{
    SetValue(v);
}

DAVA_VIRTUAL_REFLECTION_IMPL(FilePathComponentValue)
{
    ReflectionRegistrator<FilePathComponentValue>::Begin(CreateComponentStructureWrapper<FilePathComponentValue>())
    .Field("value", &FilePathComponentValue::GetFilePath, &FilePathComponentValue::SetFilePath)[M::ProxyMetaRequire()]
    .End();
}

} // namespace TArc
} // namespace DAVA