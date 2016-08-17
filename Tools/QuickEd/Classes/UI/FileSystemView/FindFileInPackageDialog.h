#pragma once

#include <QDialog>

class QFileSystemModel;

class FindFileInPackageDialog : public QDialog
{
public:
    static QString GetFilePath(const QString& rootPath, QWidget* parent);

private:
    explicit FindFileInPackageDialog(const QString& rootPath, QWidget* parent = nullptr);
    void InitFromPath(const QString& path);

    QString filePath;
};
