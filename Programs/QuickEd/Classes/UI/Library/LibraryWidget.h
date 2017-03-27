#pragma once

#include <QDockWidget>
#include <QPointer>
#include "ui_LibraryWidget.h"

#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <FileSystem/FilePath.h>

class LibraryModel;
class PackageNode;

namespace DAVA
{
namespace TArc
{
class FieldBinder;
class ContextAccessor;
}
}

class LibraryWidget : public QDockWidget, public Ui::LibraryWidget
{
    Q_OBJECT

public:
    LibraryWidget(QWidget* parent = nullptr);
    ~LibraryWidget() override;

    void SetAccessor(DAVA::TArc::ContextAccessor* accessor);

private:
    void OnPackageChanged(const DAVA::Any& package);
    void OnProjectPathChanged(const DAVA::Any& projectPath);

    void BindFields();

    LibraryModel* libraryModel = nullptr;
    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
    DAVA::TArc::ContextAccessor* accessor = nullptr;
};
