#pragma once

#include "Base/BaseTypes.h"

#include <QDialog>
#include <QMap>

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

    QString filePath;
    QString projectDir;
    QMap<QString, QString> availableFiles;
    QCompleter* completer = nullptr;
};
