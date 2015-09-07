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


#ifndef __QUICKED_DOCUMENT_H__
#define __QUICKED_DOCUMENT_H__

#include <QUndoStack>
#include <QMap>
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Systems/Interfaces.h"

#include "Systems/SelectionSystem.h"
#include "Systems/CanvasSystem.h"
#include "Systems/HUDSystem.h"
#include "Systems/CursorSystem.h"
#include "Systems/TransformSystem.h"


struct WidgetContext
{
    virtual ~WidgetContext() = default;
};

namespace DAVA {
    class FilePath;
}

class PackageNode;
class QtModelPackageCommandExecutor;

class PropertiesModel;
class PackageModel;
class ControlNode;
class AbstractProperty;

class Document final : public QObject
{
    Q_OBJECT
public:
    explicit Document(PackageNode *package, QObject *parent = nullptr);
    ~Document();
    void Detach();
    void Attach();
    CanvasSystem *GetCanvasSystem();
    HUDSystem *GetHUDSystem();
    const DAVA::FilePath &GetPackageFilePath() const;
    QUndoStack *GetUndoStack() const;
    PackageNode *GetPackage() const;
    QtModelPackageCommandExecutor *GetCommandExecutor() const;
    void RefreshLayout();
    WidgetContext* GetContext(QObject* requester) const;
    void SetContext(QObject* requester, WidgetContext* widgetContext);
    void OnSelectionWasChanged(const SelectedControls &selected, const SelectedControls &deselected);
    bool OnInput(DAVA::UIEvent *currentInput);
    void GetControlNodesByPos(DAVA::Vector<ControlNode*> &controlNodes, const DAVA::Vector2 &pos) const;
    void GetControlNodesByRect(DAVA::Set<ControlNode*> &controlNodes, const DAVA::Rect &rect) const;
    ControlNode* GetControlByMenu(const DAVA::Vector<ControlNode*> &nodes, const DAVA::Vector2 &pos) const;

signals:
    void SelectedNodesChanged(const DAVA::Set<PackageBaseNode*> &selected, const DAVA::Set<PackageBaseNode*> &deselected);
public slots:
    void RefreshAllControlProperties();
    void OnSelectedNodesChanged(const DAVA::Set<PackageBaseNode*> &selected, const DAVA::Set<PackageBaseNode*> &deselected);
private:
    void SetSelectedNodes(const DAVA::Set<PackageBaseNode*> &selected, const DAVA::Set<PackageBaseNode*> &deselected);
    void GetControlNodesByPosImpl(DAVA::Vector<ControlNode*> &controlNodes, const DAVA::Vector2 &pos, ControlNode *node) const;
    void GetControlNodesByRectImpl(DAVA::Set<ControlNode*> &controlNodes, const DAVA::Rect &rect, ControlNode *node) const;
    DAVA::UnorderedMap < QObject*, WidgetContext* > contexts;
    DAVA::Set<PackageBaseNode*> selectedNodes;
    PackageNode *package = nullptr;
    QtModelPackageCommandExecutor *commandExecutor = nullptr;
    QUndoStack *undoStack = nullptr;

    SelectionSystem selectionSystem;
    CanvasSystem canvasSystem;
    HUDSystem hudSystem;
    CursorSystem cursorSystem;
    TransformSystem transformSystem;
    QList<InputInterface*> inputListeners;
    QList<BaseSystem*> systems;
};

inline QUndoStack *Document::GetUndoStack() const
{
    return undoStack;
}

inline PackageNode *Document::GetPackage() const
{
    return package;
}

inline QtModelPackageCommandExecutor *Document::GetCommandExecutor() const
{
    return commandExecutor;
}

#endif // __QUICKED_DOCUMENT_H__
