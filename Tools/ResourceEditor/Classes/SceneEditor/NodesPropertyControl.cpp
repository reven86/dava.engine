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



#include "NodesPropertyControl.h"
#include "ControlsFactory.h"

#include "DraggableDialog.h"

#include "EditorSettings.h"
#include "EditorConfig.h"
#include "SceneNodePropertyNames.h"

#include "Scene3D/Components/CustomPropertiesComponent.h"
#include "../StringConstants.h"

NodesPropertyControl::NodesPropertyControl(const Rect & rect, bool _createNodeProperties)
    :   UIControl(rect)
{
	btnPlus = 0;
	btnMinus = 0;
	propControl = 0;
	workingScene = 0;

    deletionList = NULL;
    listHolder = NULL;
    btnCancel = NULL;
    
    propertyList = NULL;

    nodesDelegate = NULL;
    currentSceneNode = NULL;
    currentDataNode = NULL;
    createNodeProperties = _createNodeProperties;

    
    Rect propertyRect(0, 0, rect.dx, rect.dy);
    
    if(!createNodeProperties)
    {
        propertyRect.dy -= ControlsFactory::BUTTON_HEIGHT;
        
        btnPlus = ControlsFactory::CreateButton(
                                                Rect(0, propertyRect.dy, 
                                                     ControlsFactory::BUTTON_HEIGHT, ControlsFactory::BUTTON_HEIGHT), 
                                                L"+");
        btnPlus->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NodesPropertyControl::OnPlus));
        AddControl(btnPlus);
        
        btnMinus = ControlsFactory::CreateButton(
                                                 Rect(ControlsFactory::BUTTON_HEIGHT, propertyRect.dy, 
                                                      ControlsFactory::BUTTON_HEIGHT, ControlsFactory::BUTTON_HEIGHT), 
                                                 L"-");
        btnMinus->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NodesPropertyControl::OnMinus));
        AddControl(btnMinus);
		
        propControl = new CreatePropertyControl(Rect(0, rect.dy - ControlsFactory::BUTTON_HEIGHT*(PROP_CONTROL_ELEM_COUNT + 1), 
                                                     rect.dx, ControlsFactory::BUTTON_HEIGHT*PROP_CONTROL_ELEM_COUNT), this);
        
        
        listHolder = new UIControl(propertyRect);
        btnCancel = ControlsFactory::CreateButton(
                                                Rect(0, propertyRect.dy - ControlsFactory::BUTTON_HEIGHT, 
                                                propertyRect.dx, ControlsFactory::BUTTON_HEIGHT), 
                                                LocalizedString(L"dialog.cancel"));
        btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NodesPropertyControl::OnCancel));
        listHolder->AddControl(btnCancel);
    }
    
    propertyList = new PropertyList(propertyRect, this);
    AddControl(propertyList);
}
    
NodesPropertyControl::~NodesPropertyControl()
{
    ReleaseChildLodData();
    
    SafeRelease(deletionList);
    SafeRelease(listHolder);
    SafeRelease(btnCancel);

    SafeRelease(propControl);
    
    SafeRelease(btnMinus);
    SafeRelease(btnPlus);

    SafeRelease(propertyList);
    
    SafeRelease(currentSceneNode);
}

void NodesPropertyControl::WillAppear()
{
}

void NodesPropertyControl::WillDisappear()
{
    LodComponent *lodComponent = GetLodComponent(currentSceneNode);
    if(lodComponent)
    {
        lodComponent->SetForceDistance(LodComponent::INVALID_DISTANCE);
    }
    else
    {
        if(propertyList->IsPropertyAvaliable(String("property.lodnode.forcedistance"))
           && propertyList->GetBoolPropertyValue(String("property.lodnode.forcedistance")))
        {
            RestoreChildLodDistances();
        }
        
        for(int32 i = 0; i < (int32)childLodComponents.size(); ++i)
        {
            childLodComponents[i]->SetForceDistance(LodComponent::INVALID_DISTANCE);
        }
        
        ReleaseChildLodData();
    }
    
    SafeRelease(currentSceneNode);
}

void NodesPropertyControl::ReadFrom(Entity *sceneNode)
{
	SafeRelease(currentSceneNode);
    currentSceneNode = SafeRetain(sceneNode);
    currentDataNode = NULL;
    ReleaseChildLodData();
    
    propertyList->ReleaseProperties();
    
    if(!createNodeProperties)
    {
        propertyList->AddSection("property.scenenode.generalc++", GetHeaderState("property.scenenode.generalc++", true));
        propertyList->AddIntProperty("property.scenenode.retaincount", PropertyList::PROPERTY_IS_READ_ONLY);
        propertyList->AddStringProperty("property.scenenode.classname", PropertyList::PROPERTY_IS_READ_ONLY);
        propertyList->AddStringProperty("property.scenenode.c++classname", PropertyList::PROPERTY_IS_READ_ONLY);
        propertyList->AddStringProperty("property.scenenode.ptr", PropertyList::PROPERTY_IS_READ_ONLY);
        
//        for (uint32 k = 0; k < Component::COMPONENT_COUNT; ++k)
//        {
//            propertyList->AddStringProperty(Format("Component:%s", sceneNode->components sa M.  zz))
//        }
        
        
        
        propertyList->SetIntPropertyValue("property.scenenode.retaincount", sceneNode->GetRetainCount());
        propertyList->SetStringPropertyValue("property.scenenode.classname", sceneNode->GetClassName());
        propertyList->SetStringPropertyValue("property.scenenode.c++classname", typeid(*sceneNode).name());
        propertyList->SetStringPropertyValue("property.scenenode.ptr", Format("%p", sceneNode));
        
        AABBox3 unitBox = sceneNode->GetWTMaximumBoundingBoxSlow();
        if((-AABBOX_INFINITY != unitBox.max.x) && (AABBOX_INFINITY != unitBox.min.x))
        {
            propertyList->AddSubsection(String("Unit size"));

            Vector3 size = unitBox.max - unitBox.min;
            
            propertyList->AddFloatProperty(String("X-Size"), PropertyList::PROPERTY_IS_READ_ONLY);
            propertyList->SetFloatPropertyValue(String("X-Size"), size.x);
            
            propertyList->AddFloatProperty(String("Y-Size"), PropertyList::PROPERTY_IS_READ_ONLY);
            propertyList->SetFloatPropertyValue(String("Y-Size"), size.y);

            propertyList->AddFloatProperty(String("Z-Size"), PropertyList::PROPERTY_IS_READ_ONLY);
            propertyList->SetFloatPropertyValue(String("Z-Size"), size.z);
            
        }
    }

    propertyList->AddSection("property.scenenode.scenenode", GetHeaderState("property.scenenode.scenenode", true));
    propertyList->AddStringProperty(SCENE_NODE_NAME_PROPERTY_NAME, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetStringPropertyValue(SCENE_NODE_NAME_PROPERTY_NAME, sceneNode->GetName());

    if(!createNodeProperties)
    {
        propertyList->AddBoolProperty(SCENE_NODE_IS_VISIBLE_PROPERTY_NAME, PropertyList::PROPERTY_IS_EDITABLE);
		propertyList->SetBoolPropertyValue(SCENE_NODE_IS_VISIBLE_PROPERTY_NAME, sceneNode->GetVisible());
		
        propertyList->AddSection("property.scenenode.matrixes", GetHeaderState("property.scenenode.matrixes", false));
        propertyList->AddMatrix4Property("property.scenenode.localmatrix", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddMatrix4Property("property.scenenode.worldmatrix", PropertyList::PROPERTY_IS_READ_ONLY);

        propertyList->SetMatrix4PropertyValue("property.scenenode.localmatrix", sceneNode->GetLocalTransform());
        propertyList->SetMatrix4PropertyValue("property.scenenode.worldmatrix", sceneNode->GetWorldTransform());
    }

    
	{ //static light
		 propertyList->AddSection("Used in static lighting", GetHeaderState("Used in static lighting", true));

		 propertyList->AddBoolProperty(SCENE_NODE_USED_IN_STATIC_LIGHTING_PROPERTY_NAME);
		 propertyList->SetBoolPropertyValue(SCENE_NODE_USED_IN_STATIC_LIGHTING_PROPERTY_NAME, sceneNode->GetCustomProperties()->GetBool("editor.staticlight.used", true));

		 propertyList->AddBoolProperty(SCENE_NODE_CAST_SHADOWS_PROPERTY_NAME);
		 propertyList->SetBoolPropertyValue(SCENE_NODE_CAST_SHADOWS_PROPERTY_NAME, sceneNode->GetCustomProperties()->GetBool("editor.staticlight.castshadows", true));

		 propertyList->AddBoolProperty(SCENE_NODE_RECEIVE_SHADOWS_PROPERTY_NAME);
		 propertyList->SetBoolPropertyValue(SCENE_NODE_RECEIVE_SHADOWS_PROPERTY_NAME, sceneNode->GetCustomProperties()->GetBool("editor.staticlight.receiveshadows", true));
	}
    
    
    { // LodNodes at Hierarchy
        LodComponent *lodComponent = GetLodComponent(currentSceneNode);
        if(!lodComponent)
        {
            AddChildLodSection();
        }        
    }
    
    
    //must be last
    if(!createNodeProperties)
    {
        propertyList->AddSection("property.scenenode.customproperties", GetHeaderState("property.scenenode.customproperties", true));
        
        KeyedArchive *customProperties = sceneNode->GetCustomProperties();
        Map<String, VariantType*> propsData = customProperties->GetArchieveData();
        for (Map<String, VariantType*>::iterator it = propsData.begin(); it != propsData.end(); ++it)
        {
            String name = it->first;
            VariantType * key = it->second;

			if(EditorConfig::Instance()->HasProperty(name))
			{
				EditorConfig::Instance()->AddPropertyEditor(propertyList, name, key);
			}
			else
			{
				switch (key->type) 
				{
					case VariantType::TYPE_BOOLEAN:
						propertyList->AddBoolProperty(name, PropertyList::PROPERTY_IS_EDITABLE);
						propertyList->SetBoolPropertyValue(name, key->AsBool());
						break;
                    
					case VariantType::TYPE_STRING:
						propertyList->AddStringProperty(name, PropertyList::PROPERTY_IS_EDITABLE);
						propertyList->SetStringPropertyValue(name, key->AsString());
						break;

					case VariantType::TYPE_INT32:
						propertyList->AddIntProperty(name, PropertyList::PROPERTY_IS_EDITABLE);
						propertyList->SetIntPropertyValue(name, key->AsInt32());
						break;

					case VariantType::TYPE_FLOAT:
						propertyList->AddFloatProperty(name, PropertyList::PROPERTY_IS_EDITABLE);
						propertyList->SetFloatPropertyValue(name, key->AsFloat());
						break;
                    
					default:
						break;
				}
			}
        }
    }
    else
    {
        KeyedArchive *customProperties = sceneNode->GetCustomProperties();
        if(customProperties && customProperties->IsKeyExists(ResourceEditor::EDITOR_IS_LOCKED))
        {
            propertyList->AddSection("property.scenenode.customproperties", GetHeaderState("property.scenenode.customproperties", true));
            propertyList->AddBoolProperty(ResourceEditor::EDITOR_IS_LOCKED, PropertyList::PROPERTY_IS_EDITABLE);
            propertyList->SetBoolPropertyValue(ResourceEditor::EDITOR_IS_LOCKED,
												customProperties->GetBool(ResourceEditor::EDITOR_IS_LOCKED));
        }
    }
}

void NodesPropertyControl::AddChildLodSection()
{
    DVASSERT(0 == childLodComponents.size());
    DVASSERT(0 == childDistances.size());
    
    Vector<Entity *>nodes;
    currentSceneNode->GetChildNodes(nodes);
    nodes.push_back(currentSceneNode);
    for(int32 i = 0; i < (int32)nodes.size(); ++i)
    {
        LodComponent *lodComponent = GetLodComponent(nodes[i]);
        if(lodComponent)
        {
            childLodComponents.push_back(lodComponent);
        }
    }
    
    if(0 < childLodComponents.size())
    {
        propertyList->AddSection("LODs at hierarchy", GetHeaderState("LODs at hierarchy", true));
        
        propertyList->AddBoolProperty("property.lodnode.forcedistance");
        propertyList->SetBoolPropertyValue("property.lodnode.forcedistance", false);
        propertyList->AddSliderProperty("property.lodnode.distanceslider", false);
        propertyList->SetSliderPropertyValue("property.lodnode.distanceslider", 0, 
                                             LodComponent::MAX_LOD_DISTANCE, LodComponent::MIN_LOD_DISTANCE);
        
        
        
        struct LodInfo
        {
            float32 distance;
            int32 triangles;
            int32 count;
            
            LodInfo()
            {
                count = 0;
                distance = 0.0f;
                triangles = 0;
            }
            
        }lodInfo[LodComponent::MAX_LOD_LAYERS];
        
        for(int32 i = 0; i < (int32)childLodComponents.size(); ++i)
        {
            float32 *distances = new float32[LodComponent::MAX_LOD_LAYERS];
            
            
            Vector<LodComponent::LodData*> lodLayers;
            childLodComponents[i]->GetLodData(lodLayers);
            Vector<LodComponent::LodData*>::const_iterator lodLayerIt = lodLayers.begin();
            
            int32 iLod = 0;
            for(; iLod < childLodComponents[i]->GetLodLayersCount(); ++iLod)
            {
                //TODO: calculate triangles
                LodComponent::LodData *layer = *lodLayerIt;
                lodInfo[iLod].triangles += GetTrianglesForLodLayer(layer);
                ++lodLayerIt;

                distances[iLod] = childLodComponents[i]->GetLodLayerDistance(iLod);
                
                lodInfo[iLod].distance += childLodComponents[i]->GetLodLayerDistance(iLod);
                lodInfo[iLod].count++;
            }
            
            for(; iLod < LodComponent::MAX_LOD_LAYERS; ++iLod)
            {
                distances[iLod] = 0.0f;
            }
            
            childDistances.push_back(distances);
        }
        
        propertyList->AddDistanceProperty("property.lodnode.distances");
        float32 *distances = new float32[LodComponent::MAX_LOD_LAYERS];
        int32 *triangles = new int32[LodComponent::MAX_LOD_LAYERS];
        int32 count = 0;
        for(int32 iLod = 0; iLod < LodComponent::MAX_LOD_LAYERS; ++iLod)
        {
            if(lodInfo[iLod].count)
            {
                triangles[iLod] = lodInfo[iLod].triangles;
                distances[iLod] = lodInfo[iLod].distance / lodInfo[iLod].count;
                count = iLod + 1;
            }
            else 
            {
                distances[iLod] = 0.0f;
                triangles[iLod] = 0;
            }
            
        }
        
        propertyList->SetDistancePropertyValue("property.lodnode.distances", distances, triangles, count);
        SafeDeleteArray(distances);
        SafeDeleteArray(triangles);
        
        propertyList->AddMessageProperty("Set Distances", 
                                         Message(this, &NodesPropertyControl::OnSetDistancesForLodNodes));
    }
}

void NodesPropertyControl::ReleaseChildLodData()
{
    for(int32 i = 0; i < (int32)childDistances.size(); ++i)
    {
        SafeDeleteArray(childDistances[i]);
    }
    childDistances.clear();
    childLodComponents.clear();
}

void NodesPropertyControl::ReadFrom(DataNode *dataNode)
{
    currentSceneNode = NULL;
    currentDataNode = dataNode;
    
    propertyList->ReleaseProperties();
    if(!createNodeProperties)
    {
        propertyList->AddSection("property.scenenode.generalc++", GetHeaderState("property.scenenode.generalc++", true));
        propertyList->AddIntProperty("property.scenenode.retaincount", PropertyList::PROPERTY_IS_READ_ONLY);
        propertyList->AddStringProperty("property.scenenode.classname", PropertyList::PROPERTY_IS_READ_ONLY);
        propertyList->AddStringProperty("property.scenenode.c++classname", PropertyList::PROPERTY_IS_READ_ONLY);
        
        propertyList->SetIntPropertyValue("property.scenenode.retaincount", dataNode->GetRetainCount());
        propertyList->SetStringPropertyValue("property.scenenode.classname", dataNode->GetClassName());
        propertyList->SetStringPropertyValue("property.scenenode.c++classname", typeid(*dataNode).name());
    }
}


void NodesPropertyControl::OnDistancePropertyChanged(PropertyList *, const String &forKey, float32 newValue, int32 index)
{
    if("property.lodnode.distances" == forKey)
    {
        LodComponent *lodComponent = GetLodComponent(currentSceneNode);
        if(lodComponent)
        {
            lodComponent->SetLodLayerDistance(index, newValue);
        }
        else 
        {
            if(propertyList->GetBoolPropertyValue("property.lodnode.forcedistance"))
            {
                for(int32 i = 0; i < (int32)childLodComponents.size(); ++i)
                {
                    if(index < childLodComponents[i]->GetLodLayersCount())
                    {
                        childLodComponents[i]->SetLodLayerDistance(index, newValue);
                    }
                }
            }
        }     
    }
    
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged(forKey);
	}
}


void NodesPropertyControl::OnSliderPropertyChanged(PropertyList *, const String &forKey, float32 newValue)
{
    if("property.lodnode.distanceslider" == forKey)
    {
        if(propertyList->GetBoolPropertyValue("property.lodnode.forcedistance"))
        {
            LodComponent *lodComponent = GetLodComponent(currentSceneNode);
            if(lodComponent)
            {
                lodComponent->SetForceDistance(newValue);
            }
            else
            {
                for(int32 i = 0; i < (int32)childLodComponents.size(); ++i)
                {
                    childLodComponents[i]->SetForceDistance(newValue);
                }
            }     
        }
    }

    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged(forKey);
	}
}


void NodesPropertyControl::SetDelegate(NodesPropertyDelegate *delegate)
{
    nodesDelegate = delegate;
}

void NodesPropertyControl::OnStringPropertyChanged(PropertyList *, const String &forKey, const String &newValue)
{
	bool needUpdatePropertyPanel = false;

    if(forKey == SCENE_NODE_NAME_PROPERTY_NAME) //Entity
    {
        if(currentSceneNode)
        {
            currentSceneNode->SetName(newValue);
        }
        else if(currentDataNode)
        {
            currentDataNode->SetName(newValue);
        }
    }
    else if(!createNodeProperties)
    {
        if(currentSceneNode)
        {
            KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
            if(customProperties->IsKeyExists(forKey))
            {
                customProperties->SetString(forKey, newValue);
				needUpdatePropertyPanel = true;
            }
        }
    }
    
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged(forKey);
	}

	if(needUpdatePropertyPanel)
		UpdateFieldsForCurrentNode();
}
void NodesPropertyControl::OnFloatPropertyChanged(PropertyList *, const String &forKey, float newValue)
{
    if(!createNodeProperties)
    {
        if(currentSceneNode)
        {
            KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
            if(customProperties->IsKeyExists(forKey))
            {
                customProperties->SetFloat(forKey, newValue);
            }
        }
    }

    
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged(forKey);
	}

	UpdateFieldsForCurrentNode();
}
void NodesPropertyControl::OnIntPropertyChanged(PropertyList *, const String &forKey, int newValue)
{
	bool needUpdatePropertyPanel = false;

    if(!createNodeProperties)
    {
        if(currentSceneNode)
        {
            KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
            if(customProperties->IsKeyExists(forKey))
            {
                customProperties->SetInt32(forKey, newValue);
				needUpdatePropertyPanel = true;
            }
        }
    }

    
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged(forKey);
	}

	if(needUpdatePropertyPanel)
		UpdateFieldsForCurrentNode();
}

void NodesPropertyControl::OnBoolPropertyChanged(PropertyList *, const String &forKey, bool newValue)
{
	bool needUpdatePropertyPanel = true;
    if(currentSceneNode)
    {
        KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
        
        if(SCENE_NODE_USED_IN_STATIC_LIGHTING_PROPERTY_NAME == forKey)
        {
            customProperties->SetBool("editor.staticlight.used", newValue);
        }
		else if(SCENE_NODE_CAST_SHADOWS_PROPERTY_NAME == forKey)
		{
			customProperties->SetBool("editor.staticlight.castshadows", newValue);
		}
		else if(SCENE_NODE_RECEIVE_SHADOWS_PROPERTY_NAME == forKey)
		{
			customProperties->SetBool("editor.staticlight.receiveshadows", newValue);
		}
        else if("property.lodnode.forcedistance" == forKey)
        {
            float32 forceDistance = (newValue)  ? propertyList->GetSliderPropertyValue("property.lodnode.distanceslider")
                                                : LodComponent::INVALID_DISTANCE;
            
            LodComponent *lodComponent = GetLodComponent(currentSceneNode);
            if(lodComponent)
            {
                lodComponent->SetForceDistance(forceDistance);
            }
            else 
            {
                if(newValue)    SetChildLodDistances();
                else            RestoreChildLodDistances();
                    
                for(int32 i = 0; i < (int32)childLodComponents.size(); ++i)
                {
                    childLodComponents[i]->SetForceDistance(forceDistance);
                }
            }

			needUpdatePropertyPanel = false;
        }
// 		else if("CollisionFlag" == forKey)
// 		{
// 			currentSceneNode->PropagateBoolProperty("CollisionFlag", newValue);
// 		}
        
        if(!createNodeProperties)
        {
            KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
            if (forKey == SCENE_NODE_IS_VISIBLE_PROPERTY_NAME)
            {
                currentSceneNode->SetVisible(newValue);
            }
            
            if(customProperties->IsKeyExists(forKey))
            {
                customProperties->SetBool(forKey, newValue);
            }
        }
        else
        {
            KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
            if(customProperties->IsKeyExists(forKey))
            {
                customProperties->SetBool(forKey, newValue);
            }
        }
    }

    
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged(forKey);
	}

	if(needUpdatePropertyPanel)
		UpdateFieldsForCurrentNode();
}
void NodesPropertyControl::OnFilepathPropertyChanged(PropertyList *, const String &forKey, const FilePath &)
{
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged(forKey);
	}
}
void NodesPropertyControl::OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey)
{
	if(!createNodeProperties)
    {
        if(currentSceneNode)
        {
            KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
            if(customProperties->IsKeyExists(forKey))
            {
                customProperties->SetInt32(forKey, newItemIndex);
            }
        }
    }

    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged(forKey);
	}

	UpdateFieldsForCurrentNode();
}

void NodesPropertyControl::OnMatrix4Changed(PropertyList *, const String &forKey, const Matrix4 & matrix4)
{
    if(forKey == "property.scenenode.localmatrix")
    {
        if(currentSceneNode)
        {
            currentSceneNode->SetLocalTransform(matrix4);
        }
    }
    
    
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged(forKey);
	}
}

void NodesPropertyControl::OnSectionExpanded(PropertyList *, const String &forKey, bool isExpanded)
{
    SetHeaderState(forKey, isExpanded);
}


void NodesPropertyControl::OnPlus(BaseObject * , void * , void * )
{
    if(propControl->GetParent() || listHolder->GetParent())
    {
        return;
    }

    AddControl(propControl);
}

void NodesPropertyControl::OnMinus(BaseObject * , void * , void * )
{
    if(propControl->GetParent() || listHolder->GetParent())
    {
        return;
    }
    
    if(currentSceneNode)
    {
        KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
        Map<String, VariantType*> propsData = customProperties->GetArchieveData();
        
        int32 size = propsData.size();
        if(size)
        {
            Rect r = listHolder->GetRect();
            r.dy = Min(r.dy, (float32)size * CellHeight(NULL, 0));
            r.y = listHolder->GetRect().dy - r.dy - ControlsFactory::BUTTON_HEIGHT;
            
            deletionList = new UIList(r, UIList::ORIENTATION_VERTICAL);
            ControlsFactory::SetScrollbar(deletionList);
            ControlsFactory::CustomizePropertyCell(deletionList, false);
            
            deletionList->SetDelegate(this);
            
            listHolder->AddControl(deletionList);
            deletionList->Refresh();
            AddControl(listHolder);
        }
    }
}

void NodesPropertyControl::NodeCreated(bool success, const String &name, int32 type, VariantType *defaultValue)
{
    RemoveControl(propControl);
    if(success && currentSceneNode)
    {
        KeyedArchive *currentProperties = currentSceneNode->GetCustomProperties();
        
        switch (type) 
        {
			case VariantType::TYPE_STRING:    
				if(defaultValue)
				{
					currentProperties->SetString(name, defaultValue->AsString());
				}
				else
				{
					currentProperties->SetString(name, "");
				}
                break;
			case VariantType::TYPE_INT32:  
				if(defaultValue)
				{
					currentProperties->SetInt32(name, defaultValue->AsInt32());
				}
				else
				{
					currentProperties->SetInt32(name, 0);
				}
                break;
			case VariantType::TYPE_FLOAT:
				if(defaultValue)
				{
					currentProperties->SetFloat(name, defaultValue->AsFloat());
				}
				else
				{
					currentProperties->SetFloat(name, 0.f);
				}
                break;
			case VariantType::TYPE_BOOLEAN:
				if(defaultValue)
				{
					currentProperties->SetBool(name, defaultValue->AsBool());
				}
				else
				{
					currentProperties->SetBool(name, false);
				}
                break;
            default:
                break;
        }
        
        UpdateFieldsForCurrentNode();
    }
}


int32 NodesPropertyControl::ElementsCount(UIList * )
{
    if(currentSceneNode)
    {
        KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
        Map<String, VariantType*> propsData = customProperties->GetArchieveData();
        
        return propsData.size();
    }
    
    return 0;
}

UIListCell *NodesPropertyControl::CellAtIndex(UIList *list, int32 index)
{
    UIListCell *c = (UIListCell *)list->GetReusableCell("Deletion list");
    if (!c) 
    {
        c = new UIListCell(Rect(0, 0, 200, 20), "Deletion list");
    }
    
    if(currentSceneNode)
    {
        KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
        Map<String, VariantType*> propsData = customProperties->GetArchieveData();
        int32 i = 0; 
        for (Map<String, VariantType*>::iterator it = propsData.begin(); it != propsData.end(); ++it, ++i)
        {
            if(i == index)
            {
                String name = it->first;
                
                ControlsFactory::CustomizeListCell(c, StringToWString(name));
                break;
            }
        }
    }
    
    return c;
}

int32 NodesPropertyControl::CellHeight(UIList * , int32 )
{
    return CELL_HEIGHT;
}

void NodesPropertyControl::OnCellSelected(UIList *, UIListCell *selectedCell)
{
    if(currentSceneNode)
    {
        int32 index = selectedCell->GetIndex();
        KeyedArchive *customProperties = currentSceneNode->GetCustomProperties();
        Map<String, VariantType*> propsData = customProperties->GetArchieveData();
        int32 i = 0; 
        for (Map<String, VariantType*>::iterator it = propsData.begin(); it != propsData.end(); ++it, ++i)
        {
            if(i == index)
            {
                customProperties->DeleteKey(it->first);
                
                OnCancel(NULL, NULL, NULL);
                ReadFrom(currentSceneNode);
                break;
            }
        }
    }
}

void NodesPropertyControl::OnCancel(BaseObject * , void * , void * )
{
    listHolder->RemoveControl(deletionList);
    SafeRelease(deletionList);
    RemoveControl(listHolder);
}


void NodesPropertyControl::SetWorkingScene(DAVA::Scene *scene)
{
    workingScene = scene;
}


void NodesPropertyControl::UpdateFieldsForCurrentNode()
{
    if(currentSceneNode)
    {
		currentSceneNode->Retain();
        ReadFrom(currentSceneNode);
        currentSceneNode->Release();
    }
}

void NodesPropertyControl::UpdateMatricesForCurrentNode()
{
	if(!createNodeProperties && currentSceneNode)
	{
		propertyList->SetBoolPropertyValue("property.scenenode.isVisible", currentSceneNode->GetVisible());
		propertyList->SetMatrix4PropertyValue("property.scenenode.localmatrix", currentSceneNode->GetLocalTransform());
		propertyList->SetMatrix4PropertyValue("property.scenenode.worldmatrix", currentSceneNode->GetWorldTransform());
	}
}

bool NodesPropertyControl::GetHeaderState(const String & headerName, bool defaultValue)
{
    KeyedArchive *settings = EditorSettings::Instance()->GetSettings();
    return settings->GetBool("NodesProperety." + headerName, defaultValue);
}

void NodesPropertyControl::SetHeaderState(const String & headerName, bool newState)
{
    KeyedArchive *settings = EditorSettings::Instance()->GetSettings();
    settings->SetBool("NodesProperety." + headerName, newState);
    EditorSettings::Instance()->Save();
}

void NodesPropertyControl::OnSetDistancesForLodNodes(BaseObject * , void * , void * )
{
    SetChildLodDistances();
    for(int32 i = 0; i < (int32)childLodComponents.size(); ++i)
    {
        for(int32 iLod = 0; iLod < childLodComponents[i]->GetLodLayersCount(); ++iLod)
        {
            childDistances[i][iLod] = childLodComponents[i]->GetLodLayerDistance(iLod);
        }
    }

}

void NodesPropertyControl::SetChildLodDistances()
{
    int32 count = propertyList->GetDistancePropertyCount(String("property.lodnode.distances"));
    if(count)
    {
        float32 *distances = new float32[count];
        for(int32 i = 0; i < count; ++i)
        {
            distances[i] = propertyList->GetDistancePropertyValue(String("property.lodnode.distances"), i);
        }
        
        for(int32 i = 0; i < (int32)childLodComponents.size(); ++i)
        {
            for(int32 iLod = 0; iLod < count && iLod < childLodComponents[i]->GetLodLayersCount(); ++iLod)
            {
                childLodComponents[i]->SetLodLayerDistance(iLod, distances[iLod]);
            }
        }
        
        SafeDeleteArray(distances);
    }
}

void NodesPropertyControl::RestoreChildLodDistances()
{
    for(int32 i = 0; i < (int32)childLodComponents.size(); ++i)
    {
        for(int32 iLod = 0; iLod < childLodComponents[i]->GetLodLayersCount(); ++iLod)
        {
            childLodComponents[i]->SetLodLayerDistance(iLod, childDistances[i][iLod]);
        }
    }
}

void NodesPropertyControl::SetSize(const Vector2 &newSize)
{
    UIControl::SetSize(newSize);
    
    Rect propertyRect(0, 0, newSize.x, newSize.y);
    
    if(!createNodeProperties)
    {
        propertyRect.dy -= ControlsFactory::BUTTON_HEIGHT;
        
        btnPlus->SetPosition(Vector2(0, propertyRect.dy));
        btnMinus->SetPosition(Vector2(ControlsFactory::BUTTON_HEIGHT, propertyRect.dy));

        propControl->SetPosition(Vector2(0, newSize.y - ControlsFactory::BUTTON_HEIGHT*(PROP_CONTROL_ELEM_COUNT + 1)));

        listHolder->SetSize(propertyRect.GetSize());
        

        btnCancel->SetPosition(Vector2(0, propertyRect.dy - ControlsFactory::BUTTON_HEIGHT));
    }

    
    propertyList->SetSize(propertyRect.GetSize());
}


int32 NodesPropertyControl::GetTrianglesForLodLayer(LodComponent::LodData *lodData)
{
    int32 trianglesCount = 0;
    for(int32 n = 0; n < (int32)lodData->nodes.size(); ++n)
    {
        Vector<Entity *> meshes;
        lodData->nodes[n]->GetChildNodes(meshes);
        
        meshes.push_back(lodData->nodes[n]);
        
        for(int32 m = 0; m < (int32)meshes.size(); ++m)
        {
            RenderObject *ro = GetRenderObject(meshes[m]);
            if(!ro || ro->GetType() != RenderObject::TYPE_MESH) continue;

            uint32 count = ro->GetRenderBatchCount();
            for(uint32 r = 0; r < count; ++r)
            {
                RenderBatch *batch = ro->GetRenderBatch(r);
                
                PolygonGroup *pg = batch->GetPolygonGroup();
                if(pg)
                {
                    trianglesCount += pg->GetIndexCount() / 3;
                }
            }
        }
    }
    return trianglesCount;
}


