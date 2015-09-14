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
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "SystemManager.h"
#include "Functional/Signal.h"
#include "SelectionTracker.h"
#include "Math/Rect.h"


struct WidgetContext
{
    virtual ~WidgetContext() = 0;
};

inline WidgetContext::~WidgetContext()
{
    
}

namespace DAVA {
    class FilePath;
    class UIControl;
    class UIEvent;
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
    Q_PROPERTY(int scale READ GetScale WRITE SetScale NOTIFY ScaleChanged RESET ResetScale);
    Q_PROPERTY(bool emulationMode READ IsInEmulationMode WRITE SetEmulationMode NOTIFY EmulationModeChanged)
    
public:
    explicit Document(PackageNode *package, QObject *parent = nullptr);
    ~Document();

    void Activate();
    void Deactivate();
    
    int GetScale() const;
    bool IsInEmulationMode() const;
    SystemManager *GetSystemManager();
    const DAVA::FilePath &GetPackageFilePath() const;
    QUndoStack *GetUndoStack();
    PackageNode *GetPackage();
    QtModelPackageCommandExecutor *GetCommandExecutor();
    WidgetContext* GetContext(QObject* requester) const;

    void SetContext(QObject* requester, WidgetContext* widgetContext);

    void RefreshLayout();

    void SelectControlByMenu(const DAVA::Vector<ControlNode*> &nodes, const DAVA::Vector2 &pos, ControlNode* &selectedNode);
    
signals:
    void ScaleChanged(int scale);
    void EmulationModeChanged(bool emulationMode);
    void SelectedNodesChanged(const SelectedNodes &selected, const SelectedNodes &deselected);
    void CanvasSizeChanged();
public slots:
    void SetScale(int scale);
    void ResetScale();
    void SetEmulationMode(bool emulationMode);
    void RefreshAllControlProperties();
    void SetSelectedNodes(const SelectedNodes &selected, const SelectedNodes &deselected);

private:
    static const int defaultScale = 100;
    int scale = defaultScale;

    DAVA::UnorderedMap < QObject*, WidgetContext* > contexts;

    PackageNode *package = nullptr;
    QtModelPackageCommandExecutor *commandExecutor = nullptr;
    QUndoStack *undoStack = nullptr;

    SystemManager systemManager;
    SelectionTracker selectionTracker;
};

#endif // __QUICKED_DOCUMENT_H__
