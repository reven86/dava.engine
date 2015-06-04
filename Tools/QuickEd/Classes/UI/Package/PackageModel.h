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

#include <QAbstractItemModel>
#include <QMimeData>
#include <QStringList>
#include <QUndoStack>
#include <QItemSelection>

#include "Model/PackageHierarchy/PackageListener.h"

class PackageNode;
class ControlNode;
class PackageBaseNode;
class PackageControlsNode;
class ControlsContainerNode;
class QtModelPackageCommandExecutor;

class PackageModel : public QAbstractItemModel, private PackageListener
{
    Q_OBJECT

public:
    PackageModel(PackageNode *root, QtModelPackageCommandExecutor *commandExecutor, QObject *parent = 0);
    virtual ~PackageModel();
    
    QModelIndex indexByNode(PackageBaseNode *node) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const  override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const  override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    virtual Qt::DropActions supportedDropActions() const override;
    virtual QStringList mimeTypes() const override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    
private: // PackageListener
    virtual void ControlPropertyWasChanged(ControlNode *node, AbstractProperty *property) override;

    virtual void ControlWillBeAdded(ControlNode *node, ControlsContainerNode *destination, int row) override;
    virtual void ControlWasAdded(ControlNode *node, ControlsContainerNode *destination, int row) override;
    
    virtual void ControlWillBeRemoved(ControlNode *node, ControlsContainerNode *from) override;
    virtual void ControlWasRemoved(ControlNode *node, ControlsContainerNode *from) override;

    virtual void ImportedPackageWillBeAdded(PackageControlsNode *node, PackageNode *to, int index) override;
    virtual void ImportedPackageWasAdded(PackageControlsNode *node, PackageNode *to, int index) override;
    
    virtual void ImportedPackageWillBeRemoved(PackageControlsNode *node, PackageNode *from) override;
    virtual void ImportedPackageWasRemoved(PackageControlsNode *node, PackageNode *from) override;
    
private:
    PackageNode *root;
    QtModelPackageCommandExecutor *commandExecutor;


};

#endif // __QUICKED_PACKAGE_MODEL_H__
