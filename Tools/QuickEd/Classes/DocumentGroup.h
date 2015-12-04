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


#ifndef QUICKED_DOCUMENTGROUP_H
#define QUICKED_DOCUMENTGROUP_H

#include <QObject>
#include <QUndoGroup>
#include "Base/BaseTypes.h"
#include "EditorSystems/SelectionContainer.h"

class Document;
class PackageBaseNode;

class DocumentGroup : public QObject
{
    Q_OBJECT
public:
    explicit DocumentGroup(QObject *parent = nullptr);
    ~DocumentGroup();

    void AddDocument(Document*);
    void RemoveDocument(Document*);
    QList<Document*> GetDocuments() const;
    Document* GetActiveDocument() const;
    const QUndoGroup* GetUndoGroup() const;

signals:
    void ActiveDocumentChanged(Document*);
    void DocumentActivated(Document*);
    void DocumentDeactivated(Document*);
    void SelectedNodesChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void CanvasSizeChanged();

public slots:
    void SetActiveDocument(Document* document);
    void SetSelectedNodes(const SelectedNodes& selected, const SelectedNodes& deselected);
    void SetEmulationMode(bool emulationMode);
    void SetPixelization(bool hasPixelization);
    void SetScale(float scale);
    void SetDPR(qreal dpr);
    void OnSelectAllRequested();
    void FocusNextChild();
    void FocusPreviousChild();

protected:
    bool emulationMode = false;
    bool hasPixalization = false;
    float scale = 100.0f;
    qreal dpr = 1.0f;

    Document *active;
    QList<Document*> documentList;
    QUndoGroup *undoGroup;
};

#endif // QUICKED_DOCUMENTGROUP_H
