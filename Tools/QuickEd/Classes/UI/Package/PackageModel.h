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


#ifndef __QUICKED_PACKAGE_MODEL_H__
#define __QUICKED_PACKAGE_MODEL_H__

#include "Functional/SignalBase.h"
#include <QAbstractItemModel>
#include <QMimeData>
#include "EditorSystems/SelectionContainer.h"

class AbstractProperty;
class PackageNode;
class ControlNode;
class PackageBaseNode;
class StyleSheetsNode;
class PackageControlsNode;
class ControlsContainerNode;
class ImportedPackagesNode;
class QtModelPackageCommandExecutor;

class PackageModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    PackageModel(QObject* parent = nullptr);
    void Reset(std::weak_ptr<PackageNode> package, std::weak_ptr<QtModelPackageCommandExecutor> executor);

    QModelIndex indexByNode(PackageBaseNode *node) const;
    
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const  override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const  override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

signals:
    void BeforeNodesMoved(const SelectedNodes& nodes);
    void NodesMoved(const SelectedNodes& nodes);

private: // PackageListener
    void OnControlPropertyWasChanged(ControlNode* node, AbstractProperty* property);
    void OnStylePropertyWasChanged(StyleSheetNode* node, AbstractProperty* property);

    void OnControlWillBeAdded(ControlNode* node, ControlsContainerNode* destination, int row);
    void OnControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int row);

    void OnControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from);
    void OnControlWasRemoved(ControlNode* node, ControlsContainerNode* from);

    void OnStyleWillBeAdded(StyleSheetNode* node, StyleSheetsNode* destination, int index);
    void OnStyleWasAdded(StyleSheetNode* node, StyleSheetsNode* destination, int index);

    void OnStyleWillBeRemoved(StyleSheetNode* node, StyleSheetsNode* from);
    void OnStyleWasRemoved(StyleSheetNode* node, StyleSheetsNode* from);

    void OnImportedPackageWillBeAdded(PackageNode* node, ImportedPackagesNode* to, int index);
    void OnImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index);

    void OnImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from);
    void OnImportedPackageWasRemoved(PackageNode* node, ImportedPackagesNode* from);

    int GetRowIndex(int row, const QModelIndex& parent) const;

    std::weak_ptr<PackageNode> package;
    std::weak_ptr<QtModelPackageCommandExecutor> commandExecutor;
    DAVA::TrackedObject connectionTracker;
};

#endif // __QUICKED_PACKAGE_MODEL_H__
