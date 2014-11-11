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


#include "propertygridcontainerwidget.h"
#include "ui_propertygridcontainerwidget.h"

#include "statepropertygridwidget.h"

#include "MetadataFactory.h"
#include "EditorSettings.h"

PropertyGridContainerWidget::PropertyGridContainerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PropertyGridContainerWidget),
    activeMetadata(NULL)
{
    ui->setupUi(this);
	ui->topArea->hide();

    widgetsFactory.InitializeWidgetParents(this);
    
    ConnectToSignals();
}

PropertyGridContainerWidget::~PropertyGridContainerWidget()
{
    disconnect(PropertiesGridController::Instance(), SIGNAL(PropertiesGridUpdated()),
    this, SLOT(OnPropertiesGridUpdated()));
    disconnect(PropertiesGridController::Instance(), SIGNAL(UIControlsDeselected()),
    this, SLOT(OnUIControlsDeselected()));
    delete ui;
}

void PropertyGridContainerWidget::ConnectToSignals()
{
    connect(PropertiesGridController::Instance(), SIGNAL(PropertiesGridUpdated()),
            this, SLOT(OnPropertiesGridUpdated()));
    connect(PropertiesGridController::Instance(), SIGNAL(UIControlsDeselected()),
            this, SLOT(OnUIControlsDeselected()));
}

void PropertyGridContainerWidget::OnPropertiesGridUpdated()
{
    if (PropertiesGridController::Instance()->GetActiveTreeNodesList().empty() == false)
    {
        // List of UI Controls is selected.
        BuildPropertiesGridList();
        return;
    }

    // Build the Properties Grid based on the active control.
    const HierarchyTreeNode* activeTreeNode = PropertiesGridController::Instance()->GetActiveTreeNode();
    DVASSERT(activeTreeNode);

    // This code is executed when Screen/Platform are selected.
    UIControl* activeControl = NULL;
    const HierarchyTreeControlNode* activeTreeControlNode = dynamic_cast<const HierarchyTreeControlNode*>(activeTreeNode);
    if (activeTreeControlNode)
    {
        activeControl = activeTreeControlNode->GetUIObject();
    }

    BaseMetadata* metaData = GetActiveMetadata(activeTreeNode);    
    BuildPropertiesGrid(activeControl, metaData, activeTreeNode->GetId());
}

void PropertyGridContainerWidget::OnUIControlsDeselected()
{
    // No Tree Node selected, try to build custom properties grid.
    CleanupPropertiesGrid();

    HierarchyTreeScreenNode* activeScreenNode = HierarchyTreeController::Instance()->GetActiveScreen();
    BuildCustomPropertiesGrid(activeScreenNode);
}

void PropertyGridContainerWidget::CleanupPropertiesGrid()
{
    // Cleanup the properties grid.
    for (PropertyGridWidgetsFactory::PROPERTYGRIDWIDGETSITER iter = activeWidgetsList.begin();
         iter != activeWidgetsList.end(); iter ++)
    {
        BasePropertyGridWidget* widget = (*iter);
        widget->Cleanup();

		if (!dynamic_cast<StatePropertyGridWidget*>(*iter))
		{
	        ui->scrollAreaLayout->removeWidget(widget);
		}
		else
		{
			ui->topAreaLayout->removeWidget(widget);
		}

        widget->hide();
    }

	ui->topArea->hide();

    // Also release the Active Metadata, if any.
    CleanupActiveMetadata();
}

void PropertyGridContainerWidget::BuildCustomPropertiesGrid(HierarchyTreeScreenNode* screenNode)
{
    if (!screenNode)
    {
        return;
    }

    BaseMetadata* customMetadata = MetadataFactory::Instance()->GetCustomMetadata(screenNode);
    if (!customMetadata)
    {
        return;
    }

    METADATAPARAMSVECT params;
    params.push_back(BaseMetadataParams(HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY, NULL));
    BuildPropertiesGrid(customMetadata, params);
}

void PropertyGridContainerWidget::BuildPropertiesGridList()
{
    // Need to understand which Metadata we need. Since multiple controls of different
    // type might be selected, we are looking for the most common Metadata.
    const HierarchyTreeController::SELECTEDCONTROLNODES &activeNodes = PropertiesGridController::Instance()->GetActiveTreeNodesList();
    BaseMetadata* metaData = GetActiveMetadata(activeNodes);

    METADATAPARAMSVECT params;
    for (HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = activeNodes.begin();
         iter != activeNodes.end(); iter ++)
    {
        const HierarchyTreeControlNode* controlNode = *iter;
        BaseMetadataParams controlNodeParam(controlNode->GetId(), controlNode->GetUIObject());
        params.push_back(controlNodeParam);
    }

    BuildPropertiesGrid(metaData, params);
}

void PropertyGridContainerWidget::BuildPropertiesGrid(UIControl* activeControl,
                                                      BaseMetadata* metaData,
                                                      HierarchyTreeNode::HIERARCHYTREENODEID activeNodeID)
{
    METADATAPARAMSVECT params;
    params.push_back(BaseMetadataParams(activeNodeID, activeControl));
    BuildPropertiesGrid(metaData, params);
}

void PropertyGridContainerWidget::BuildPropertiesGrid(BaseMetadata* metaData,
                                                      const METADATAPARAMSVECT& params)
{
    CleanupPropertiesGrid();

    // Metadata is state-aware.
    const Vector<UIControl::eControlState> &activeStates = PropertiesGridController::Instance()->GetActiveUIControlStates();
    metaData->SetUIControlStates(activeStates);

    this->activeWidgetsList = widgetsFactory.GetWidgets(metaData);
    for (PropertyGridWidgetsFactory::PROPERTYGRIDWIDGETSITER iter = activeWidgetsList.begin();
         iter != activeWidgetsList.end(); iter ++)
    {
		BasePropertyGridWidget* widget = (*iter);

		if (!dynamic_cast<StatePropertyGridWidget*>(*iter))
		{
			ui->scrollAreaLayout->addWidget(widget);
		}
		else
		{
			ui->topArea->show();
			ui->topAreaLayout->addWidget(widget);
		}
		widget->show();

		// Eaach widget should initialize itself by using Metadata (for property names) and
		// UI Control (for actual values).
		metaData->SetupParams(params);
		widget->Initialize(metaData);
    }
    
    // The Active Metatata is known.
    this->activeMetadata = metaData;
}

// Work with Active Metadata.
BaseMetadata* PropertyGridContainerWidget::GetActiveMetadata(const HierarchyTreeNode* activeTreeNode)
{
    if (activeTreeNode)
    {
        return MetadataFactory::Instance()->GetMetadataForTreeNode(activeTreeNode);
    }
    
    return NULL;
}

// Work with Active Metadata.
BaseMetadata* PropertyGridContainerWidget::GetActiveMetadata(const HierarchyTreeController::SELECTEDCONTROLNODES &activeNodes)
{
    return MetadataFactory::Instance()->GetMetadataForTreeNodesList(activeNodes);
}

void PropertyGridContainerWidget::CleanupActiveMetadata()
{
    SAFE_DELETE(this->activeMetadata);
}
