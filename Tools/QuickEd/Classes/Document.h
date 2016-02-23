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
#include "EditorSystems/SelectionContainer.h"

struct WidgetContext
{
    virtual ~WidgetContext() = 0;
};

inline WidgetContext::~WidgetContext()
{
}

namespace DAVA
{
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

class QFileSystemWatcher;

class Document final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canSave READ CanSave NOTIFY CanSaveChanged);

public:
    explicit Document(const DAVA::RefPtr<PackageNode>& package, QObject* parent = nullptr);
    ~Document();

    const DAVA::FilePath& GetPackageFilePath() const;
    QString GetPackageAbsolutePath() const;
    QUndoStack* GetUndoStack() const;
    PackageNode* GetPackage() const;
    QtModelPackageCommandExecutor* GetCommandExecutor() const;
    WidgetContext* GetContext(void* requester) const;

    void SetContext(void* requester, WidgetContext* widgetContext);
    void RefreshLayout();
    bool CanSave() const;
    bool IsDocumentExists() const;

signals:
    void FileChanged(Document* document);
    void CanSaveChanged(bool canSave);

public slots:
    void RefreshAllControlProperties();

private slots:
    void OnFileChanged(const QString& path);
    void OnCleanChanged(bool clean);

private:
    void SetCanSave(bool canSave);
    DAVA::UnorderedMap<void*, WidgetContext*> contexts;

    DAVA::RefPtr<PackageNode> package;
    std::unique_ptr<QtModelPackageCommandExecutor> commandExecutor;
    std::unique_ptr<QUndoStack> undoStack;
    QFileSystemWatcher* fileSystemWatcher = nullptr;
    bool fileExists = true;
    bool canSave = false;
};

#endif // __QUICKED_DOCUMENT_H__
