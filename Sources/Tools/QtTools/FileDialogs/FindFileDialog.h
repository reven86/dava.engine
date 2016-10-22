#pragma once

#include "Base/BaseTypes.h"

#include <QDialog>
#include <QMap>

namespace Ui
{
class FindFileDialog;
}

namespace DAVA
{
class FilePath;
}

class ProjectStructure;
class QFileSystemModel;
class QCompleter;
class QAction;

class FindFileDialog : public QDialog
{
public:
    static QString GetFilePath(const ProjectStructure* projectStructure, const DAVA::String& suffix, QWidget* parent);
    static QAction* CreateFindInFilesAction(QWidget* parent);

private:
    explicit FindFileDialog(const ProjectStructure* projectStructure, const DAVA::String& suffix, QWidget* parent = nullptr);

    void Init(const DAVA::Vector<DAVA::FilePath>& files);

    bool eventFilter(QObject* obj, QEvent* event);

    std::unique_ptr<Ui::FindFileDialog> ui;

    QString ToShortName(const QString& name) const;
    QString FromShortName(const QString& name) const;

    QString prefix;
    QCompleter* completer = nullptr;
};
