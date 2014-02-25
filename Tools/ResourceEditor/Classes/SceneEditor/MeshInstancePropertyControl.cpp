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



#include "MeshInstancePropertyControl.h"

#include "SceneEditorScreenMain.h"
#include "../AppScreens.h"
#include "EditorBodyControl.h"
#include "Qt/Scene/SceneDataManager.h"
#include "Qt/Scene/SceneData.h"

#include "Qt/CubemapEditor/MaterialHelper.h"

MeshInstancePropertyControl::MeshInstancePropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
}

MeshInstancePropertyControl::~MeshInstancePropertyControl()
{

}

void MeshInstancePropertyControl::ReadFrom(Entity * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

    MeshInstanceNode *mesh = dynamic_cast<MeshInstanceNode *> (sceneNode);
	DVASSERT(mesh);

    propertyList->AddSection("property.meshinstance.meshinstance", GetHeaderState("property.meshinstance.meshinstance", true));
        
    //BBOX
    AABBox3 bbox = mesh->GetBoundingBox();
    AABBox3 transformedBox;
    bbox.GetTransformedBox(mesh->GetWorldTransform(), transformedBox);
    
    propertyList->AddStringProperty("property.meshinstance.bboxmin", PropertyList::PROPERTY_IS_READ_ONLY);
    propertyList->AddStringProperty("property.meshinstance.bboxmax", PropertyList::PROPERTY_IS_READ_ONLY);
    
    propertyList->SetStringPropertyValue("property.meshinstance.bboxmin", Format("%0.2f, %0.2f, %0.2f", 
                                                            transformedBox.min.x, transformedBox.min.y, transformedBox.min.z));
    propertyList->SetStringPropertyValue("property.meshinstance.bboxmax", Format("%0.2f, %0.2f, %0.2f", 
                                                            transformedBox.max.x, transformedBox.max.y, transformedBox.max.z));
    
    materials.clear();
    materialNames.clear();

	if(workingScene)
	{
		workingScene->GetDataNodes(materials);
		//VI: remove skybox materials so they not to appear in the lists
		MaterialHelper::FilterMaterialsByType(materials, DAVA::Material::MATERIAL_SKYBOX);
	}

    int32 matCount = (int32)materials.size();
    for(int32 i = 0; i < matCount; ++i)
    {
        Material *mat = materials[i];
        materialNames.push_back(mat->GetName());
    }
    
    Vector<PolygonGroupWithMaterial*> polygroups = mesh->GetPolygonGroups();
    for(int32 i = 0; i < (int32)polygroups.size(); ++i)
    {
        PolygonGroup *pg = polygroups[i]->GetPolygonGroup();
        
        String fieldName = Format("PolygonGroup #%d", i);
        propertyList->AddSection(fieldName, GetHeaderState(fieldName, true));

        int32 vertexFormat = pg->GetFormat();
        
        String keyPrefix = Format("#%d", i);
        propertyList->AddBoolProperty(keyPrefix + ". fmt.NORMAL", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.NORMAL", 0 != (vertexFormat & EVF_NORMAL));
        
        propertyList->AddBoolProperty(keyPrefix + ". fmt.COLOR", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.COLOR", 0 != (vertexFormat & EVF_COLOR));
        
        propertyList->AddBoolProperty(keyPrefix + ". fmt.TEXCOORD0", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.TEXCOORD0", 0 != (vertexFormat & EVF_TEXCOORD0));
        
        propertyList->AddBoolProperty(keyPrefix + ". fmt.TEXCOORD1", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.TEXCOORD1", 0 != (vertexFormat & EVF_TEXCOORD1));
        
        propertyList->AddBoolProperty(keyPrefix + ". fmt.TEXCOORD2", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.TEXCOORD2", 0 != (vertexFormat & EVF_TEXCOORD2));
        
        propertyList->AddBoolProperty(keyPrefix + ". fmt.TEXCOORD3", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.TEXCOORD3", 0 != (vertexFormat & EVF_TEXCOORD3));
        
        propertyList->AddBoolProperty(keyPrefix + ". fmt.TANGENT", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.TANGENT", 0 != (vertexFormat & EVF_TANGENT));
        
        propertyList->AddBoolProperty(keyPrefix + ". fmt.BINORMAL", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.BINORMAL", 0 != (vertexFormat & EVF_BINORMAL));
        
        propertyList->AddBoolProperty(keyPrefix + ". fmt.JOINTWEIGHT", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.JOINTWEIGHT", 0 != (vertexFormat & EVF_JOINTWEIGHT));

		propertyList->AddIntProperty(keyPrefix + ".lightmap.size");
		propertyList->SetIntPropertyValue(keyPrefix + ".lightmap.size", currentSceneNode->GetCustomProperties()->GetInt32(keyPrefix + ".lightmap.size", 128));
        
        
        if(matCount && !createNodeProperties)
        {
            String comboName = keyPrefix + ". Material";
            propertyList->AddComboProperty(comboName, materialNames);
            
            if(polygroups[i]->GetMaterial())
            {
                String meshMatName = polygroups[i]->GetMaterial()->GetName();
                for(int32 iMat = 0; iMat < (int32)materials.size(); ++iMat)
                {
                    if(meshMatName == materialNames[iMat])
                    {
                        propertyList->SetComboPropertyIndex(comboName, iMat);
                        break;
                    }
                }
            }
            else
            {
                propertyList->SetComboPropertyIndex(comboName, 0);
            }
            
            propertyList->AddMessageProperty("property.meshinstance.editmaterial", 
                                             Message(this, &MeshInstancePropertyControl::OnGo2Materials, polygroups[i]->GetMaterial()));

        }
        propertyList->AddMessageProperty("property.meshinstance.showtriangles", 
                                         Message(this, &MeshInstancePropertyControl::OnShowTexture, pg));
        
    }

	propertyList->AddSection("property.meshinstance.dynamicshadow", GetHeaderState("property.meshinstance.dynamicshadow", true));
	
	propertyList->AddBoolProperty("property.meshinstance.dynamicshadow.enable");
	propertyList->SetBoolPropertyValue("property.meshinstance.dynamicshadow.enable", currentSceneNode->GetCustomProperties()->GetBool("property.meshinstance.dynamicshadow.enable", false));

	propertyList->AddMessageProperty("property.meshinstance.dynamicshadow.converttovolume", Message(this, &MeshInstancePropertyControl::OnConvertToShadowVolume));
}


void MeshInstancePropertyControl::OnGo2Materials(DAVA::BaseObject *object, void *userData, void *callerData)
{
    Material *material = (Material *)userData;
    SceneEditorScreenMain *screen = (SceneEditorScreenMain *)UIScreenManager::Instance()->GetScreen(SCREEN_MAIN);
    screen->EditMaterial(material);
}

void MeshInstancePropertyControl::OnShowTexture(DAVA::BaseObject *object, void *userData, void *callerData)
{
    PolygonGroup *polygonGroup = (PolygonGroup *)userData;
    SceneEditorScreenMain *screen = (SceneEditorScreenMain *)UIScreenManager::Instance()->GetScreen(SCREEN_MAIN);
    screen->ShowTextureTriangles(polygonGroup);
}

void MeshInstancePropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if(forKey[0] == '#')
    {
        int32 index = GetIndexFromKey(forKey);
        
        String keyPrefix = Format("#%d", index);
        int32 vertexFormat = EVF_VERTEX;

		vertexFormat |= GetFlagValue(keyPrefix + ". fmt.NORMAL", EVF_NORMAL);
		vertexFormat |= GetFlagValue(keyPrefix + ". fmt.COLOR", EVF_COLOR);
		vertexFormat |= GetFlagValue(keyPrefix + ". fmt.TEXCOORD0", EVF_TEXCOORD0);
		vertexFormat |= GetFlagValue(keyPrefix + ". fmt.TEXCOORD1", EVF_TEXCOORD1);
		vertexFormat |= GetFlagValue(keyPrefix + ". fmt.TEXCOORD2", EVF_TEXCOORD2);
		vertexFormat |= GetFlagValue(keyPrefix + ". fmt.TEXCOORD3", EVF_TEXCOORD3);
		vertexFormat |= GetFlagValue(keyPrefix + ". fmt.TANGENT", EVF_TANGENT);
		vertexFormat |= GetFlagValue(keyPrefix + ". fmt.BINORMAL", EVF_BINORMAL);
		vertexFormat |= GetFlagValue(keyPrefix + ". fmt.JOINTWEIGHT", EVF_JOINTWEIGHT);

		//TODO: need to save new vertex format
    }

	if(forKey == "property.meshinstance.dynamicshadow.enable")
	{
		currentSceneNode->GetCustomProperties()->SetBool(forKey, newValue);
		if(newValue)
		{
			((MeshInstanceNode*)currentSceneNode)->CreateDynamicShadowNode();
		}
		else
		{
			((MeshInstanceNode*)currentSceneNode)->DeleteDynamicShadowNode();
		}
	}

    NodesPropertyControl::OnBoolPropertyChanged(forList, forKey, newValue);
}

int32 MeshInstancePropertyControl::GetFlagValue(const String &keyName, int32 flagValue)
{
	if(propertyList->GetBoolPropertyValue(keyName))
		return flagValue;

	return 0;
}

void MeshInstancePropertyControl::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
	if(forKey[0] == '#')
	{
		int32 index = GetIndexFromKey(forKey);

		String keyPrefix = Format("#%d", index);
		currentSceneNode->GetCustomProperties()->SetInt32(keyPrefix + ".lightmap.size", newValue);
	}

	NodesPropertyControl::OnIntPropertyChanged(forList, forKey, newValue);
}

void MeshInstancePropertyControl::OnComboIndexChanged(PropertyList *forList, const String &forKey, 
                                               int32 newItemIndex, const String &newItemKey)
{
	if(forKey[0] == '#')
    {
        int32 index = GetIndexFromKey(forKey);

        MeshInstanceNode *mesh = dynamic_cast<MeshInstanceNode *> (currentSceneNode);
        mesh->ReplaceMaterial(materials[newItemIndex], index);
        
        UpdateFieldsForCurrentNode();
    }

    NodesPropertyControl::OnComboIndexChanged(forList, forKey, newItemIndex, newItemKey);
}

int32 MeshInstancePropertyControl::GetIndexFromKey(const String &forKey)
{
	String num = forKey.substr(1);
    int32 retNum = atoi(num.c_str());
    
    return retNum;
}

void MeshInstancePropertyControl::OnConvertToShadowVolume(BaseObject * object, void * userData, void * callerData)
{
	((MeshInstanceNode*)currentSceneNode)->ConvertToShadowVolume();

	SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
	activeScene->RemoveSceneNode(currentSceneNode);
}
