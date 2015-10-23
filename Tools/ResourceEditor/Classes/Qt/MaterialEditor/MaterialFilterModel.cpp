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


#include "MaterialFilterModel.h"
#include "MaterialModel.h"
#include "MaterialItem.h"

#include "Scene/SceneEditor2.h"
#include "Scene/EntityGroup.h"
#include "Tools/MimeData/MimeDataHelper2.h"
#include "Commands2/MaterialSwitchParentCommand.h"

#include "Scene3D/Scene.h"

#include <QTimer>
#include <QDebug>


MaterialFilteringModel::MaterialFilteringModel(MaterialModel *_materialModel, QObject *parent /* = NULL */)
    : QSortFilterProxyModel(parent)
    , materialModel(_materialModel)
    , filterType( SHOW_ALL )
{
	setSourceModel(materialModel);
}

void MaterialFilteringModel::Sync()
{
	materialModel->Sync();
}

void MaterialFilteringModel::SetScene(SceneEditor2 * scene)
{
	materialModel->SetScene(scene);
}

SceneEditor2* MaterialFilteringModel::GetScene()
{
    return materialModel->GetScene();
}

void MaterialFilteringModel::SetSelection(const EntityGroup *group)
{
	materialModel->SetSelection(group);
}

DAVA::NMaterial * MaterialFilteringModel::GetMaterial(const QModelIndex & index) const
{
	return materialModel->GetMaterial(mapToSource(index));
}

QModelIndex MaterialFilteringModel::GetIndex(DAVA::NMaterial *material, const QModelIndex &parent /*= QModelIndex()*/) const
{
	return mapFromSource(materialModel->GetIndex(material, mapFromSource(parent)));
}

bool MaterialFilteringModel::dropCanBeAccepted(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	QModelIndex target = mapToSource(index(row, column, parent));
    return materialModel->dropCanBeAccepted(data, action, target.row(), target.column(), target.parent());
}

void MaterialFilteringModel::setFilterType(int type)
{
    if ( type == filterType )
        return ;

    filterType = static_cast< eFilterType >( type );
    invalidate();
    //invalidateFilter();
}

int MaterialFilteringModel::getFilterType() const
{
    return filterType;
}

bool MaterialFilteringModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	MaterialItem *item = (MaterialItem *)materialModel->itemFromIndex(materialModel->index(sourceRow, 0, sourceParent));
    if ( !item )
        return false;

    bool isSelected = item->GetFlag(MaterialItem::IS_PART_OF_SELECTION);
    bool isMaterial = false;

    DAVA::NMaterial *material = item->GetMaterial();
    if( nullptr == material->GetParent() ||
        materialModel->GetGlobalMaterial() == material->GetParent())
    {
        isMaterial = true;
    }
    switch ( filterType )
    {
    case SHOW_ALL:
        return true;

    case SHOW_NOTHING:
        return false;

    case SHOW_INSTANCES_AND_MATERIALS:
        return isMaterial || isSelected;

    case SHOW_ONLY_INSTANCES:
        {
            if ( !isMaterial )
                return isSelected;
            const int n = item->rowCount();
            for ( int i = 0; i < n; i++ )
            {
                MaterialItem *childItem = (MaterialItem*)item->child( i, 0 );
                if ( !childItem )
                    continue;
                const bool isSelected = childItem->GetFlag( MaterialItem::IS_PART_OF_SELECTION );
                if ( isSelected )
                    return true;
            }

            return false;
        }

    default:
        break;
    }

	return false;
}

bool MaterialFilteringModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    // global material should always be first
    NMaterial *mLeft = materialModel->GetMaterial(left);
    NMaterial *mRight = materialModel->GetMaterial(right);
    bool swap = QSortFilterProxyModel::lessThan(left, right);

    if ( (mLeft == NULL) || (mRight == NULL) )
        return swap;

    if (mLeft == materialModel->GetGlobalMaterial())
    {
        swap = (sortOrder() == Qt::AscendingOrder);
    }
    else if (mRight == materialModel->GetGlobalMaterial())
    {
        swap = (sortOrder() == Qt::DescendingOrder);
    }
    else
    {
        MaterialItem *lhsItem = materialModel->itemFromIndex(left.sibling(left.row(), 0));
        MaterialItem *rhsItem = materialModel->itemFromIndex(right.sibling(right.row(), 0));

        int compResult = 0;

        switch ( sortColumn() )
        {
        case MaterialModel::TITLE_COLUMN:
            compResult = compareNames(mLeft, mRight);
            break;
        case MaterialModel::LOD_COLUMN:
            compResult = lhsItem->GetLodIndex() - rhsItem->GetLodIndex();
            break;
        case MaterialModel::SWITCH_COLUMN:
            compResult = lhsItem->GetSwitchIndex() - rhsItem->GetSwitchIndex();
            break;
        default:
            break;
        }

        // If sorting column data is equal then sort by text
        if ( compResult == 0 && sortColumn() != MaterialModel::TITLE_COLUMN )
        {
            const int textComp = compareNames(mLeft, mRight);
            compResult = textComp;
        }

        swap = (compResult < 0);
    }

    return swap;
}

bool MaterialFilteringModel::dropMimeData(QMimeData const* data, Qt::DropAction action, int row, int column, QModelIndex const& parent)
{
    const bool ret = QSortFilterProxyModel::dropMimeData( data, action, row, column, parent );
    if ( ret )
        invalidate();

    return ret;
}

int MaterialFilteringModel::compareNames(DAVA::NMaterial* left, DAVA::NMaterial* right) const
{
    const QString lhsText = QString(left->GetMaterialName().c_str());
    const QString rhsText = QString(right->GetMaterialName().c_str());
    const int textComp = lhsText.compare( rhsText, Qt::CaseInsensitive );

    return textComp;
}
