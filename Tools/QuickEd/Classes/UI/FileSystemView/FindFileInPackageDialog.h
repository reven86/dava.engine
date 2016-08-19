#pragma once

#include <QDialog>

class QFileSystemModel;
class QCompleter;

class FindFileInPackageDialog : public QDialog
{
public:
    static QString GetFilePath(const QString& rootPath, QWidget* parent);
private:
    explicit FindFileInPackageDialog(const QString& rootPath, QWidget* parent = nullptr);
    void InitFromPath(const QString& path);
    bool eventFilter(QObject* obj, QEvent* event);

    QString filePath;
    QMap<QString, QString> availableFiles;
    QCompleter* completer = nullptr;
};
