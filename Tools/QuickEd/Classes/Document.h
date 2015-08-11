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
#include "Defines.h"

struct WidgetContext
{
    
};

namespace DAVA {
    class FilePath;
}

class PackageNode;
class QtModelPackageCommandExecutor;

class PropertiesModel;
class PackageModel;
class ControlNode;
class DocumentGroup;

class Document : public QObject, public SelectionInterface
{
    Q_OBJECT
    friend class DocumentGroup;
public:
    Document(PackageNode *package, QObject *parent = nullptr);
    ~Document();
    const DAVA::FilePath &GetPackageFilePath() const;
    PackageNode *GetPackage() const;

    QUndoStack *GetUndoStack() const;
    QtModelPackageCommandExecutor *GetCommandExecutor() const;

    void RefreshLayout();
    WidgetContext* GetContext(QObject* requester) const;
    void SetContext(QObject* requester, WidgetContext* widgetContext);
    void SelectionWasChanged(const SelectedControls &selected, const SelectedControls &deselected) override;
signals:
    void SelectedNodesChanged(const SelectedNodes &selected, const SelectedNodes &deselected);
public slots:
    void RefreshAllControlProperties();
    void OnSelectedNodesChanged(const SelectedNodes &selected, const SelectedNodes &deselected);
private:
    QMap < QObject*, WidgetContext* > contexts;
    SelectedNodes selectedNodes;
    PackageNode *package = nullptr;
    QtModelPackageCommandExecutor *commandExecutor = nullptr;
    QUndoStack *undoStack = nullptr;
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
