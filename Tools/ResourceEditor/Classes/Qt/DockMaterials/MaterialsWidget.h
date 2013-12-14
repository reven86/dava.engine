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



#ifndef __MATERIALS_WIDGET_H__
#define __MATERIALS_WIDGET_H__

#include <QWidget>
#include <QItemSelection>

class QVBoxLayout;
class QToolBar;
class QListView;
class QAction;
class QLineEdit;
class QLabel;
class QSortFilterProxyModel;

class SimpleMaterialModel;
class SceneEditor2;
class MaterialsWidget : public QWidget
{
	Q_OBJECT

public:
	MaterialsWidget(QWidget *parent = 0);
	~MaterialsWidget();

    void SetupSignals();
    
protected slots:

    void SceneActivated(SceneEditor2 *scene);
    void SceneDeactivated(SceneEditor2 *scene);
    
    void ViewAsList();
    void ViewAsTiles();
    
	void SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void ShowContextMenu(const QPoint & point);

	void SetFilter(const QString &filter);
    void ResetFilter();
    
    void OnEdit();
    void OnAssignToSelection();
    
private:
    
    void SetupToolbar();
    void SetupView();
    void SetupLayout();
    
    void SwitchListAndLabel();
    
    void Invalidate();
    
private:

    QVBoxLayout *layout;
    
    QToolBar *toolbar;
    QListView *materialsView;
    
    QLineEdit *searchFilter;
    
    QLabel * notFoundMessage;
    
    QAction *actionViewAsList;
    QAction *actionViewAsTiles;

    SimpleMaterialModel *materialsModel;
    QSortFilterProxyModel *proxyModel;
    
    SceneEditor2 *curScene;
};

#endif // __MATERIALS_WIDGET_H__
