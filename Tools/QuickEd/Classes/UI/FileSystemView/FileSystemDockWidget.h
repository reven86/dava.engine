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
private:
    void RefreshActions(const QModelIndexList &indexList);
    void RefreshAction(QAction *action, bool enabled, bool visible);

signals:
    void OpenPackageFile(const QString &path);

private slots:
    void OnSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onDoubleClicked(const QModelIndex &index);
    void setFilterFixedString(const QString &filterStr);
    void onDataChanged( const QModelIndex & topLeft, const QModelIndex & bottomRight );
    void customContextMenuRequested(const QPoint &pos);
    void onNewFolder();
    void onNewFile(bool checked);
    void onDeleteFile(bool checked);
    void onReloadFile(bool checked);

private:
    Ui::FileSystemDockWidget *ui;
    QFileSystemModel *model;
    QAction *newFolderAction;
    QAction *newFileAction;
    QAction *delFileAction;
};

#endif // __UI_EDITOR_FILE_SYSTEM_TREE_WIDGET_H__
