#include "TArc/Controls/PropertyPanel/Private/FilePathComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/ComponentStructureWrapper.h"
#include "TArc/Controls/PropertyPanel/Private/PropertyPanelMeta.h"
#include "TArc/Controls/FilePathEdit.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
namespace TArc
{
FilePathComponentValue::FilePathComponentValue(const Params& params_)
    : params(params_)
{
}

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
    p.ui = params.ui;
    p.wndKey = params.wndKey;
    p.fields[FilePathEdit::Fields::Value] = "value";
    p.fields[FilePathEdit::Fields::IsReadOnly] = readOnlyFieldName;
    p.fields[FilePathEdit::Fields::DialogTitle] = "dlgTitle";
    p.fields[FilePathEdit::Fields::Filters] = "filters";
    p.fields[FilePathEdit::Fields::RootDirectory] = "roorDir";
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

const String& FilePathComponentValue::GetDialogTitle() const
{
    return params.dialogTitle;
}

const String& FilePathComponentValue::GetFilters() const
{
    return params.filters;
}

const DAVA::FilePath& FilePathComponentValue::GetRootDir() const
{
    return params.rootDir;
}

DAVA_VIRTUAL_REFLECTION_IMPL(FilePathComponentValue)
{
    ReflectionRegistrator<FilePathComponentValue>::Begin(CreateComponentStructureWrapper<FilePathComponentValue>())
    .Field("value", &FilePathComponentValue::GetFilePath, &FilePathComponentValue::SetFilePath)[M::ProxyMetaRequire()]
    .Field("dlgTitle", &FilePathComponentValue::GetDialogTitle, nullptr)
    .Field("filters", &FilePathComponentValue::GetFilters, nullptr)
    .Field("roorDir", &FilePathComponentValue::GetRootDir, nullptr)
    .End();
}

} // namespace TArc
} // namespace DAVA