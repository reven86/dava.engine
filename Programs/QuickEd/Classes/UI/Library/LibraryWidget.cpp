#include "LibraryWidget.h"
#include "LibraryModel.h"

#include "Application/QEGlobal.h"

#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/ProjectModule/ProjectData.h"

#include <TArc/Core/FieldBinder.h>

#include <Base/Any.h>
#include <Reflection/ReflectedTypeDB.h>
#include <UI/UIControl.h>

LibraryWidget::LibraryWidget(QWidget* parent)
    : QDockWidget(parent)
    , libraryModel(new LibraryModel(this))
{
    setupUi(this);
    treeView->setModel(libraryModel);

    BindFields();
}

LibraryWidget::~LibraryWidget() = default;

void LibraryWidget::OnPackageChanged(const DAVA::Any& packageValue)
{
    PackageNode* package = nullptr;
    if (packageValue.CanGet<PackageNode*>())
    {
        package = packageValue.Get<PackageNode*>();
    }

    treeView->setEnabled(package != nullptr);

    libraryModel->SetPackageNode(package);

    treeView->expandAll();
    treeView->collapse(libraryModel->GetDefaultControlsModelIndex());
}

void LibraryWidget::OnProjectPathChanged(const DAVA::Any& projectPath)
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    if (projectPath.Cast<FilePath>(FilePath()).IsEmpty())
    {
        libraryModel->SetProjectLibraries(DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>(), DAVA::Vector<DAVA::FilePath>());
    }
    else
    {
        ProjectData* projectData = QEGlobal::GetDataNode<ProjectData>();
        DVASSERT(projectData != nullptr);
        DAVA::Vector<DAVA::FilePath> libraryPackages;
        for (const auto& resDir : projectData->GetLibraryPackages())
        {
            libraryPackages.push_back(resDir.absolute);
        }
        libraryModel->SetProjectLibraries(projectData->GetPrototypes(), libraryPackages);
    }

}

void LibraryWidget::BindFields()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    fieldBinder.reset(new FieldBinder(QEGlobal::GetAccessor()));

    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &LibraryWidget::OnPackageChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<ProjectData>();
        fieldDescr.fieldName = FastName(ProjectData::projectPathPropertyName);
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &LibraryWidget::OnProjectPathChanged));
    }
}
