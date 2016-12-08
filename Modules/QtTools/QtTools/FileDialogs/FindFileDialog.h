#pragma once

#include "Base/BaseTypes.h"
#include "Base/Introspection.h"

#include <QDialog>
#include <QMap>
#include <memory>

namespace Ui
{
class FindFileDialog;
}

class FileSystemCache;
class QCompleter;
class QAction;

class FindFileDialog : public QDialog, public DAVA::InspBase
{
public:
    static QString GetFilePath(const FileSystemCache* fileSystemCache, const QString& extension, QWidget* parent);
    static QAction* CreateFindInFilesAction(QWidget* parent);

    //destructor is public to resolve warnings in introspection
    ~FindFileDialog();

private:
    explicit FindFileDialog(const FileSystemCache* projectStructure, const QString& extension, QWidget* parent = nullptr);

    void Init(const QStringList& files);

    bool eventFilter(QObject* obj, QEvent* event) override;

    QString GetCommonParent(const QString& path1, const QString& path2);

    QString ToShortName(const QString& name) const;
    QString FromShortName(const QString& name) const;

    std::unique_ptr<Ui::FindFileDialog> ui;

    QString prefix;
    QCompleter* completer = nullptr;
    DAVA::String lastUsedPath;
    QStringList stringsToDisplay;

public:
    INTROSPECTION(FindFileDialog,
                  MEMBER(lastUsedPath, "FindFileDialog/lastUsedPath", DAVA::I_PREFERENCE)
                  );
};
