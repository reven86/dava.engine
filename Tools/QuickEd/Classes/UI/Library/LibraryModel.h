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


#ifndef __UI_EDITOR_LIBRARY_MODEL_H__
#define __UI_EDITOR_LIBRARY_MODEL_H__

#include <QStandardItemModel>
#include "Base/BaseTypes.h"
#include "Functional/SignalBase.h"

class PackageNode;
class PackageBaseNode;
class AbstractProperty;
class ControlNode;
class ControlsContainerNode;
class ImportedPackagesNode;

class LibraryModel : public QStandardItemModel
{
    Q_OBJECT
    enum
    {
        POINTER_DATA = Qt::UserRole + 1,
        INNER_NAME_DATA
    };
public:
    LibraryModel(QObject* parent = nullptr);
    ~LibraryModel() override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    void SetPackageNode(std::weak_ptr<PackageNode> package);

private:
    QModelIndex indexByNode(const void *node, const QStandardItem *item) const;
    void BuildModel();
    void AddControl(ControlNode* node);
    void AddImportedControl(PackageNode* node);
    void CreateControlsRootItem();
    void CreateImportPackagesRootItem();

    //Package Signals
    void OnControlPropertyWasChanged(ControlNode* node, AbstractProperty* property);
    void OnControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int row);
    void OnControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from);
    void OnImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index);
    void OnImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from);

    std::weak_ptr<PackageNode> package;
    QStandardItem *defaultControlsRootItem, *controlsRootItem, *importedPackageRootItem;
    DAVA::Vector<ControlNode*> defaultControls;
    DAVA::TrackedObject signalsTracker;
};

#endif // __UI_EDITOR_LIBRARY_MODEL_H__
