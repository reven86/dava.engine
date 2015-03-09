//
//  FileSystemTreeWidget.h
//  UIEditor
//
//  Created by Dmitry Belsky on 9.9.14.
//
//

#ifndef __UI_EDITOR_FILE_SYSTEM_TREE_WIDGET_H__
#define __UI_EDITOR_FILE_SYSTEM_TREE_WIDGET_H__

#include <QDockWidget>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>

namespace Ui {
    class FileSystemDockWidget;
}

class QFileSystemModel;

class FileSystemDockWidget : public QDockWidget
{
    Q_OBJECT
    
public:
    explicit FileSystemDockWidget(QWidget *parent = 0);
    virtual ~FileSystemDockWidget();

    void SetProjectDir(const QString &path);
signals:
    void OpenPackageFile(const QString &path);

private slots:
    void onDoubleClicked(const QModelIndex &index);
    void setFilterFixedString(const QString &filterStr);
    void onDataChanged( const QModelIndex & topLeft, const QModelIndex & bottomRight );
    
private:
    Ui::FileSystemDockWidget *ui;
    QFileSystemModel *model;
};

#endif // __UI_EDITOR_FILE_SYSTEM_TREE_WIDGET_H__
