#pragma once

#include <TArc/Controls/PropertyPanel/PropertyModelExtensions.h>

namespace DAVA
{
namespace TArc
{
class UI;
class ContextAccessor;
} // namespace TArc
} // namespace DAVA

class FilePathEditorCreator : public DAVA::TArc::EditorComponentExtension
{
public:
    FilePathEditorCreator(DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::UI* ui);
    std::unique_ptr<DAVA::TArc::BaseComponentValue> GetEditor(const std::shared_ptr<const DAVA::TArc::PropertyNode>& node) const;

private:
    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::TArc::UI* ui = nullptr;
};
