/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



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
    explicit LibraryTreeView(QWidget *parent = 0): QTreeView(parent) {};
    
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
	LibraryWidget(QWidget *parent = 0);
	~LibraryWidget();

    void SetupSignals();
    
protected slots:

    void ProjectOpened(const QString &path);
	void ProjectClosed();

    void ViewAsList();
    void ViewDetailed();
    
	void SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void ShowContextMenu(const QPoint & point);
    void fileDoubleClicked(const QModelIndex & index);
    
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
    void ShowPreview(const QString & pathname) const;
    
    QStringList GetExtensions(DAVA::ImageFormat imageFormat) const;
    
private:

    QVBoxLayout *layout;
    
    QToolBar *toolbar;
    QTreeView *filesView;
    
	QComboBox *filesTypeFilter;
    
    QAction *actionViewAsList;
    QAction *actionViewDetailed;

    
    QString rootPathname;
    LibraryFileSystemModel *filesModel;
    
    eViewMode viewMode;
	int curTypeIndex;
};

#endif // __LIBRARY_WIDGET_H__
