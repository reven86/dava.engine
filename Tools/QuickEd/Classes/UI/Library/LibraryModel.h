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
#include "Model/PackageHierarchy/PackageListener.h"

class PackageNode;
class PackageBaseNode;

class LibraryModel : public QStandardItemModel, private PackageListener
{
    Q_OBJECT
    enum
    {
        POINTER_DATA = Qt::UserRole + 1,
        INNER_NAME_DATA
    };
public:
    LibraryModel(PackageNode *root, QObject *parent = nullptr);
    ~LibraryModel() override;
   
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
private:
    QModelIndex indexByNode(const void *node, const QStandardItem *item) const;
    void BuildModel();
    void AddControl(ControlNode* node);
    void AddImportedControl(PackageNode* node);
    void CreateControlsRootItem();
    void CreateImportPackagesRootItem();
    
private:
    PackageNode *root;
    QStandardItem *defaultControlsRootItem, *controlsRootItem, *importedPackageRootItem;
    DAVA::Vector<ControlNode*> defaultControls;

private: // PackageListener
    void ControlPropertyWasChanged(ControlNode *node, AbstractProperty *property) override;
    void ControlWasAdded(ControlNode *node, ControlsContainerNode *destination, int row) override;
    void ControlWillBeRemoved(ControlNode *node, ControlsContainerNode *from) override;
    void ImportedPackageWasAdded(PackageNode *node, ImportedPackagesNode *to, int index) override;
    void ImportedPackageWillBeRemoved(PackageNode *node, ImportedPackagesNode *from) override;

};

#endif // __UI_EDITOR_LIBRARY_MODEL_H__
