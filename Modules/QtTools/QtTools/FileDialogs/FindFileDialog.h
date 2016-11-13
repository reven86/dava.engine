#pragma once

#include "Base/BaseTypes.h"
#include "Base/Introspection.h"

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

class FindFileDialog : public QDialog, public DAVA::InspBase
{
public:
    static QString GetFilePath(const ProjectStructure* projectStructure, const DAVA::String& suffix, QWidget* parent);
    static QAction* CreateFindInFilesAction(QWidget* parent);

    //destructor is public to resolve warnings in introspection
    ~FindFileDialog();

private:
    explicit FindFileDialog(const ProjectStructure* projectStructure, const DAVA::String& suffix, QWidget* parent = nullptr);

    void Init(const DAVA::Vector<DAVA::FilePath>& files);

    bool eventFilter(QObject* obj, QEvent* event) override;

    std::unique_ptr<Ui::FindFileDialog> ui;

    QString ToShortName(const QString& name) const;
    QString FromShortName(const QString& name) const;

    QString prefix;
    QCompleter* completer = nullptr;
    DAVA::String lastUsedPath;
    QStringList stringsToDisplay;

public:
    INTROSPECTION(FindFileDialog,
                  MEMBER(lastUsedPath, "FindFileDialog/lastUsedPath", DAVA::I_PREFERENCE)
                  );
};
