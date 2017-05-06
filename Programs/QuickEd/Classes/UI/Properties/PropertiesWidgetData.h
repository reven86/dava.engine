#pragma once

#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "EditorSystems/SelectionContainer.h"

#include <TArc/DataProcessing/DataNode.h>

class PropertiesWidgetData : public DAVA::TArc::DataNode
{
private:
    friend class PropertiesWidget;

    //we store selection ONLY to have difference between old selection and new selection
    //all other usages of this field is deprecated
    SelectedNodes cachedSelection;
    DAVA::List<PackageBaseNode*> selectionHistory;
    DAVA_VIRTUAL_REFLECTION(PropertiesWidgetData, DAVA::TArc::DataNode);
};
