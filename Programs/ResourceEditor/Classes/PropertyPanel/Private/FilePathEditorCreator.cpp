#include "Classes/PropertyPanel/Private/FilePathEditorCreator.h"
#include "Classes/Project/ProjectManagerData.h"
#include "Classes/Qt/Tools/PathDescriptor/PathDescriptor.h"
#include "Classes/Application/REGlobal.h"

#include <TArc/Controls/PropertyPanel/BaseComponentValue.h>
#include <TArc/Controls/PropertyPanel/Private/FilePathComponentValue.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/DataProcessing/DataContext.h>

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <FileSystem/FilePath.h>
#include <FileSystem/FileSystem.h>

namespace FilePathEditorCreatorDetail
{
DAVA::FilePath GetProjectRoot(DAVA::TArc::ContextAccessor* accessor)
{
    using namespace DAVA::TArc;
    DataContext* ctx = accessor->GetGlobalContext();
    const ProjectManagerData* data = ctx->GetData<ProjectManagerData>();
    DAVA::FilePath projectRoot = data->GetDataSource3DPath();

    const DAVA::EngineContext* engineContext = DAVA::GetEngineContext();
    if (engineContext->fileSystem->Exists(projectRoot))
    {
        return projectRoot;
    }

    return DAVA::FilePath("");
}
}

FilePathEditorCreator::FilePathEditorCreator(DAVA::TArc::ContextAccessor* accessor_, DAVA::TArc::UI* ui_)
    : accessor(accessor_)
    , ui(ui_)
{
}

std::unique_ptr<DAVA::TArc::BaseComponentValue> FilePathEditorCreator::GetEditor(const std::shared_ptr<const DAVA::TArc::PropertyNode>& node) const
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    const Type* type = node->field.ref.GetValueType()->Decay();
    static const Type* filePathType = Type::Instance<FilePath>();

    if (type == filePathType)
    {
        QString fieldName = node->field.key.Cast<QString>();
        PathDescriptor* pathDescriptor = &PathDescriptor::descriptors[0];
        for (PathDescriptor& descr : PathDescriptor::descriptors)
        {
            if (descr.pathName == fieldName)
            {
                pathDescriptor = &descr;
                break;
            }
        }

        FilePathComponentValue::Params params;
        params.ui = ui;
        params.dialogTitle = "Open File";
        params.filters = pathDescriptor->fileFilter.toStdString();
        params.wndKey = REGlobal::MainWindowKey;
        params.rootDir = FilePathEditorCreatorDetail::GetProjectRoot(accessor);
        return std::make_unique<FilePathComponentValue>(params);
    }

    return EditorComponentExtension::GetEditor(node);
}
