#ifndef __LIBRARY_WIDGET_H__
#define __LIBRARY_WIDGET_H__

#include "Render/RenderBase.h"

#include <QWidget>
#include <QTreeView>
#include <QItemSelection>
#include <QStringList>

class QVBoxLayout;
class QToolBar;
class QAction;
class QLineEdit;
class QComboBox;
class QProgressBar;
class QLabel;
class QSpacerItem;

class LibraryFileSystemModel;
class LibraryFilteringModel;

//TreeView with custom signals
class LibraryTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit LibraryTreeView(QWidget* parent = 0)
        : QTreeView(parent){};

signals:
    void DragStarted();

protected:
    virtual void startDrag(Qt::DropActions supportedActions)
    {
        emit DragStarted();
        QTreeView::startDrag(supportedActions);
    }
};

class LibraryWidget : public QWidget
{
    Q_OBJECT

    enum eViewMode
    {
        VIEW_AS_LIST = 0,
        VIEW_DETAILED
    };

public:
    LibraryWidget(QWidget* parent = 0);
    ~LibraryWidget();

    void SetupSignals();

protected slots:

    void ProjectOpened(const QString& path);
    void ProjectClosed();

    void ViewAsList();
    void ViewDetailed();

    void SelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void ShowContextMenu(const QPoint& point);
    void fileDoubleClicked(const QModelIndex& index);

    void OnFilesTypeChanged(int typeIndex);

    void OnAddModel();
    void OnEditModel();
    void OnConvertDae();
    void OnEditTextureDescriptor();
    void OnRevealAtFolder();

    void OnTreeDragStarted();

private:
    void SetupFileTypes();
    void SetupToolbar();
    void SetupView();
    void SetupLayout();

    void HideDetailedColumnsAtFilesView(bool show);

    void HidePreview() const;
    void ShowPreview(const QString& pathname) const;

    QStringList GetExtensions(DAVA::ImageFormat imageFormat) const;

private:
    QVBoxLayout* layout;

    QToolBar* toolbar;
    QTreeView* filesView;

    QComboBox* filesTypeFilter;

    QAction* actionViewAsList;
    QAction* actionViewDetailed;

    QString rootPathname;
    LibraryFileSystemModel* filesModel;

    eViewMode viewMode;
    int curTypeIndex;
};

#endif // __LIBRARY_WIDGET_H__
