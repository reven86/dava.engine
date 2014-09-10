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


#ifndef PROPERTYGRIDCONTAINERWIDGET_H
#define PROPERTYGRIDCONTAINERWIDGET_H

#include <QWidget>

#include "PropertiesGridController.h"
#include "PropertyGridWidgetsFactory.h"

namespace Ui {
class PropertyGridContainerWidget;
}

class PropertyGridContainerWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit PropertyGridContainerWidget(QWidget *parent = 0);
    ~PropertyGridContainerWidget();
    
public slots:
    // Emitted by Controller when Properties Grid is updated.
    void OnPropertiesGridUpdated();
    
    // Emitted by Controller when no UI Controls selected.
    void OnUIControlsDeselected();

protected:
    // Connect/disconnect to the signals.
    void ConnectToSignals();

    // Build the Properties Grid for the single control and for the vector of controls.
    void BuildPropertiesGrid(UIControl* activeControl, BaseMetadata* metaData,
                             HierarchyTreeNode::HIERARCHYTREENODEID activeNodeID);
    void BuildPropertiesGrid(BaseMetadata* metaData,
                             const METADATAPARAMSVECT& params);

    // Build the Properties Grid for the controls selected.
    void BuildPropertiesGridList();
    
    // Cleanup the existing Properties Grid.
    void CleanupPropertiesGrid();

    // Build the custom Properties Grid for items which aren't bound to any UI Control.
    void BuildCustomPropertiesGrid(HierarchyTreeScreenNode* activeScreenNode);

    // Work with Active Metadata.
    BaseMetadata* GetActiveMetadata(const HierarchyTreeNode* activeTreeNode);
    BaseMetadata* GetActiveMetadata(const HierarchyTreeController::SELECTEDCONTROLNODES activeNodes);

    void CleanupActiveMetadata();

private:
    Ui::PropertyGridContainerWidget *ui;
    
    // Properties Grid Widgets Factory.
    PropertyGridWidgetsFactory widgetsFactory;
    
    // Active Property Grid Widgets.
    PropertyGridWidgetsFactory::PROPERTYGRIDWIDGETSLIST activeWidgetsList;
    
    // Active Metadata.
    BaseMetadata* activeMetadata;
};


#endif // PROPERTYGRIDCONTAINERWIDGET_H
