#pragma once

#include "Base/BaseTypes.h"

#include <QDialog>
#include <QMap>

namespace Ui
{
class FindFileInPackageDialog;
}

namespace DAVA
{
class FilePath;
}

class ProjectStructure;
class QFileSystemModel;
class QCompleter;
class QAction;

class FindFileInPackageDialog : public QDialog
{
public:
    static QString GetFilePath(const ProjectStructure* projectStructure, const DAVA::String& suffix, QWidget* parent);
    static QAction* CreateFindInFilesAction(QWidget* parent);

private:
    explicit FindFileInPackageDialog(const DAVA::Vector<DAVA::FilePath>& files, QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

    void Init(const DAVA::Vector<DAVA::FilePath>& files);

    bool eventFilter(QObject* obj, QEvent* event);

    std::unique_ptr<Ui::FindFileInPackageDialog> ui;

    QString prefix;
    QCompleter* completer = nullptr;
};
