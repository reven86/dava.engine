#pragma once

#include "Base/BaseTypes.h"

#include <QDialog>
#include <QMap>

namespace Ui
{
class FindFileInPackageDialog;
}

class QFileSystemModel;
class QCompleter;

class FindFileInPackageDialog : public QDialog
{
public:
    static QString GetFilePath(const DAVA::String& rootPath, const DAVA::Vector<DAVA::String>& files, QWidget* parent);

private:
    explicit FindFileInPackageDialog(const DAVA::String& rootPath, const DAVA::Vector<DAVA::String>& files, QWidget* parent = nullptr);
    bool eventFilter(QObject* obj, QEvent* event);

    void Init(const DAVA::Vector<DAVA::String>& files);

    QString projectDir;
    QMap<QString, QString> availableFiles;
    QCompleter* completer = nullptr;
    std::unique_ptr<Ui::FindFileInPackageDialog> ui;
};
