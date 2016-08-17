#pragma once

#include <QDialog>

class QFileSystemModel;

class FindFileInPackageDialog : public QDialog
{
public:
    static QString GetFilePath();

private:
    explicit FindFileInPackageDialog(const QFileSystemModel* fileSystemModel, QWidget* parent = nullptr);
    QString filePath;
};
