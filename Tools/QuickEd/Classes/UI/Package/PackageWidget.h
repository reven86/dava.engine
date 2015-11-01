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


#ifndef __UI_EDITOR_UI_PACKAGE_WIDGET__
#define __UI_EDITOR_UI_PACKAGE_WIDGET__

#include <QWidget>
#include <QDockWidget>
#include "EditorSystems/SelectionContainer.h"
#include "Base/BaseTypes.h"
#include "ui_PackageWidget.h"

class Document;
class ControlNode;
class StyleSheetNode;
class PackageNode;
class PackageBaseNode;
class FilteredPackageModel;
class PackageModel;
class QItemSelection;

class PackageWidget : public QDockWidget, public Ui::PackageWidget
{
    Q_OBJECT
public:
    explicit PackageWidget(QWidget *parent = 0);
    ~PackageWidget() = default;

    using ExpandedIndexes = QModelIndexList ;

signals:
    void SelectedNodesChanged(const SelectedNodes& selected, const SelectedNodes& deselected);

public slots:
    void OnDocumentChanged(Document* context);
    void SetSelectedNodes(const SelectedNodes& selected, const SelectedNodes& deselected);

private slots:
    void OnSelectionChanged(const QItemSelection& proxySelected, const QItemSelection& proxyDeselected);
    void filterTextChanged(const QString&);
    void OnImport();
    void OnCopy();
    void OnPaste();
    void OnCut();
    void OnDelete();
    void OnRename();
    void OnAddStyle();
    void OnMoveUp();
    void OnMoveDown();

private:
    void MoveNode(bool up);
    void CreateActions();
    void PlaceActions();
    void LoadContext();
    void SaveContext();
    void RefreshActions();

    void SelectNode(PackageBaseNode* node);

    void CollectSelectedControls(DAVA::Vector<ControlNode*> &nodes, bool forCopy, bool forRemove);
    void CollectSelectedStyles(DAVA::Vector<StyleSheetNode*> &nodes, bool forCopy, bool forRemove);
    void CollectSelectedImportedPackages(DAVA::Vector<PackageNode*> &nodes, bool forCopy, bool forRemove);
    void CopyNodesToClipboard(const DAVA::Vector<ControlNode*> &controls, const DAVA::Vector<StyleSheetNode*> &styles);

    template <typename NodeType>
    void CollectSelectedNodes(const QItemSelection &selected, DAVA::Vector<NodeType*> &nodes, bool forCopy, bool forRemove);

    ExpandedIndexes GetExpandedIndexes() const;
    void RestoreExpandedIndexes(const ExpandedIndexes &indexes);

private:
    Document* document = nullptr;
    QAction* importPackageAction = nullptr;
    QAction* copyAction = nullptr;
    QAction* pasteAction = nullptr;
    QAction* cutAction = nullptr;
    QAction* delAction = nullptr;
    QAction* renameAction = nullptr;
    QAction* addStyleAction = nullptr;

    QAction* moveUpAction = nullptr;
    QAction* moveDownAction = nullptr;

    FilteredPackageModel* filteredPackageModel = nullptr;
    PackageModel* packageModel = nullptr;

    QString lastFilterText;
    ExpandedIndexes expandedIndexes;

    SelectionContainer selectionContainer;
};

#endif // __UI_EDITOR_UI_PACKAGE_WIDGET__
