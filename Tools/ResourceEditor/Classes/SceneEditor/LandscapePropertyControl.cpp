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



#include "LandscapePropertyControl.h"
#include "EditorSettings.h"
#include "SceneValidator.h"
#include "ControlsFactory.h"
#include "Render/Highlevel/Heightmap.h"

#include "../Qt/Main/QtUtils.h"
#include "SceneValidator.h"
#include "Scene3D/Components/DebugRenderComponent.h"

LandscapePropertyControl::LandscapePropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
    tiledModes.push_back("Tile mask mode");
    tiledModes.push_back("Texture mode");
    tiledModes.push_back("Mixed mode");
    tiledModes.push_back("Detail mask mode");
}

LandscapePropertyControl::~LandscapePropertyControl()
{

}

void LandscapePropertyControl::ReadFrom(Entity * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

	Landscape *landscape = GetLandscape();
	if (!landscape)
		return;

    propertyList->AddSection("property.landscape.landscape", GetHeaderState("property.landscape.landscape", true));
    
    propertyList->AddFloatProperty("property.landscape.size", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.landscape.height", PropertyList::PROPERTY_IS_EDITABLE); 
    Vector3 size(445.0f, 445.0f, 50.f);
    AABBox3 bbox = landscape->GetBoundingBox();
    AABBox3 emptyBox;
    if((emptyBox.min != bbox.min) && (emptyBox.max != bbox.max))
    {
        AABBox3 transformedBox = bbox;
		if(NULL != landscape->GetWorldTransformPtr())
		{
			bbox.GetTransformedBox(*landscape->GetWorldTransformPtr(), transformedBox);
		}
        size = transformedBox.max - transformedBox.min;
    }
    propertyList->SetFloatPropertyValue("property.landscape.size", size.x);
    propertyList->SetFloatPropertyValue("property.landscape.height", size.z);

    propertyList->AddComboProperty("property.landscape.tilemode", tiledModes);
    propertyList->SetComboPropertyIndex("property.landscape.tilemode", landscape->GetTiledShaderMode());

    propertyList->AddFilepathProperty("property.landscape.heightmap", ".png;.heightmap", false, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetFilepathPropertyValue("property.landscape.heightmap", landscape->GetHeightmapPathname());
    
    
    propertyList->AddSubsection("property.landscape.subsection.textures");
    AddFilepathProperty(String("property.landscape.texture.color"), TextureDescriptor::GetSupportedTextureExtensions(), Landscape::TEXTURE_COLOR);
    AddFilepathProperty(String("property.landscape.texture.tile0"), TextureDescriptor::GetSupportedTextureExtensions(), Landscape::TEXTURE_TILE0);
    AddFilepathProperty(String("property.landscape.texture.tile1"), TextureDescriptor::GetSupportedTextureExtensions(), Landscape::TEXTURE_TILE1);
    AddFilepathProperty(String("property.landscape.texture.tile2"), TextureDescriptor::GetSupportedTextureExtensions(), Landscape::TEXTURE_TILE2);
    AddFilepathProperty(String("property.landscape.texture.tile3"), TextureDescriptor::GetSupportedTextureExtensions(), Landscape::TEXTURE_TILE3);
    AddFilepathProperty(String("property.landscape.texture.tilemask"), TextureDescriptor::GetSupportedTextureExtensions(), Landscape::TEXTURE_TILE_MASK);
    AddFilepathProperty(String("property.landscape.texture.tiledtexture"), TextureDescriptor::GetSupportedTextureExtensions(), Landscape::TEXTURE_TILE_FULL);

    propertyList->AddColorProperty("property.landscape.texture.tilecolor0");
    propertyList->SetColorPropertyValue("property.landscape.texture.tilecolor0", landscape->GetTileColor(Landscape::TEXTURE_TILE0));
    
    propertyList->AddColorProperty("property.landscape.texture.tilecolor1");
    propertyList->SetColorPropertyValue("property.landscape.texture.tilecolor1", landscape->GetTileColor(Landscape::TEXTURE_TILE1));
    
    propertyList->AddColorProperty("property.landscape.texture.tilecolor2");
    propertyList->SetColorPropertyValue("property.landscape.texture.tilecolor2", landscape->GetTileColor(Landscape::TEXTURE_TILE2));
    
    propertyList->AddColorProperty("property.landscape.texture.tilecolor3");
    propertyList->SetColorPropertyValue("property.landscape.texture.tilecolor3", landscape->GetTileColor(Landscape::TEXTURE_TILE3));

    propertyList->AddMessageProperty(String("property.landscape.generatefulltiled"),
                                     Message(this, &LandscapePropertyControl::GenerateFullTiledTexture));

    propertyList->AddMessageProperty(String("property.landscape.saveheightmaptopng"),
                                     Message(this, &LandscapePropertyControl::SaveHeightmapToPng));


    propertyList->AddBoolProperty("property.landscape.showgrid", PropertyList::PROPERTY_IS_EDITABLE);
    
    // RETURN TO THIS CODE LATER
    // bool showGrid =  (0 != (landscape->GetDebugFlags() & DebugRenderComponent::DEBUG_DRAW_GRID));
    bool showGrid = false;
    propertyList->SetBoolPropertyValue("property.landscape.showgrid", showGrid);
    

	propertyList->AddIntProperty("lightmap.size");
	propertyList->SetIntPropertyValue("lightmap.size", currentSceneNode->GetCustomProperties()->GetInt32("lightmap.size", 1024));

    propertyList->AddFloatProperty("property.landscape.texture0.tilex");
    propertyList->SetFloatPropertyValue("property.landscape.texture0.tilex", landscape->GetTextureTiling(Landscape::TEXTURE_TILE0).x);
    propertyList->AddFloatProperty("property.landscape.texture0.tiley");
    propertyList->SetFloatPropertyValue("property.landscape.texture0.tiley", landscape->GetTextureTiling(Landscape::TEXTURE_TILE0).y);
    
    propertyList->AddFloatProperty("property.landscape.texture1.tilex");
    propertyList->SetFloatPropertyValue("property.landscape.texture1.tilex", landscape->GetTextureTiling(Landscape::TEXTURE_TILE1).x);
    propertyList->AddFloatProperty("property.landscape.texture1.tiley");
    propertyList->SetFloatPropertyValue("property.landscape.texture1.tiley", landscape->GetTextureTiling(Landscape::TEXTURE_TILE1).y);

    propertyList->AddFloatProperty("property.landscape.texture2.tilex");
    propertyList->SetFloatPropertyValue("property.landscape.texture2.tilex", landscape->GetTextureTiling(Landscape::TEXTURE_TILE2).x);
    propertyList->AddFloatProperty("property.landscape.texture2.tiley");
    propertyList->SetFloatPropertyValue("property.landscape.texture2.tiley", landscape->GetTextureTiling(Landscape::TEXTURE_TILE2).y);
    
    propertyList->AddFloatProperty("property.landscape.texture3.tilex");
    propertyList->SetFloatPropertyValue("property.landscape.texture3.tilex", landscape->GetTextureTiling(Landscape::TEXTURE_TILE3).x);
    propertyList->AddFloatProperty("property.landscape.texture3.tiley");
    propertyList->SetFloatPropertyValue("property.landscape.texture3.tiley", landscape->GetTextureTiling(Landscape::TEXTURE_TILE3).y);
    
    ControlsFactory::AddFogSubsection(propertyList, landscape->IsFogEnabled(), landscape->GetFogDensity(), landscape->GetFogColor());
}

void LandscapePropertyControl::AddFilepathProperty(const String &key, const String &filter, Landscape::eTextureLevel level)
{
	Landscape *landscape = GetLandscape();
	if (!landscape)
		return;
    Texture * t = landscape->GetTexture(level);
    
    propertyList->AddFilepathProperty(key, filter, true, PropertyList::PROPERTY_IS_EDITABLE);
    if(t && !t->isRenderTarget)
    {
        propertyList->SetFilepathPropertyValue(key, landscape->GetTextureName(level));
    }
    else 
    {
        propertyList->SetFilepathPropertyValue(key, String(""));
    }
}


void LandscapePropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if("property.landscape.size" == forKey || "property.landscape.height" == forKey)
    {
		Landscape *landscape = GetLandscape();
		if (landscape)
		{
			Vector3 size(
						 propertyList->GetFloatPropertyValue("property.landscape.size"),
						 propertyList->GetFloatPropertyValue("property.landscape.size"),
						 propertyList->GetFloatPropertyValue("property.landscape.height"));
			AABBox3 bbox;
			bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
			bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));
			
			Set<String> errorsLog;
			FilePath heightMap = propertyList->GetFilepathPropertyValue("property.landscape.heightmap");
			if(!SceneValidator::Instance()->ValidateHeightmapPathname(heightMap, errorsLog))
			{
				heightMap = FilePath();
			}

			landscape->BuildLandscapeFromHeightmapImage(heightMap, bbox);

			// let transform system recalc worldtranform for landscape entity
			if(NULL != currentSceneNode->GetScene())
			{
				currentSceneNode->GetScene()->transformSystem->Process(.001f);
			}
        }
    }
    if ("property.landscape.texture0.tilex" == forKey || "property.landscape.texture0.tiley" == forKey)
    {
		Landscape *landscape = GetLandscape();
		if (landscape)
		{
			Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture0.tilex"),
						   propertyList->GetFloatPropertyValue("property.landscape.texture0.tiley"));
			landscape->SetTextureTiling(Landscape::TEXTURE_TILE0, tiling);
		}
    }
    if ("property.landscape.texture1.tilex" == forKey || "property.landscape.texture1.tiley" == forKey)
    {
		Landscape *landscape = GetLandscape();
		if (landscape)
		{
			Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture1.tilex"),
						   propertyList->GetFloatPropertyValue("property.landscape.texture1.tiley"));
			landscape->SetTextureTiling(Landscape::TEXTURE_TILE1, tiling);
		}
    }
	if ("property.landscape.texture2.tilex" == forKey || "property.landscape.texture2.tiley" == forKey)
	{
		Landscape *landscape = GetLandscape();
		if (landscape)
		{
			Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture2.tilex"),
				propertyList->GetFloatPropertyValue("property.landscape.texture2.tiley"));
			landscape->SetTextureTiling(Landscape::TEXTURE_TILE2, tiling);
		}
	}
	if ("property.landscape.texture3.tilex" == forKey || "property.landscape.texture3.tiley" == forKey)
	{
		Landscape *landscape = GetLandscape();
		if (landscape)
		{
			Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture3.tilex"),
				propertyList->GetFloatPropertyValue("property.landscape.texture3.tiley"));
			landscape->SetTextureTiling(Landscape::TEXTURE_TILE3, tiling);
		}
	}
    
    if ("property.material.dencity" == forKey)
    {
		Landscape *landscape = GetLandscape();
		if (landscape)
			landscape->SetFogDensity(newValue);
    }

    NodesPropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
}


void LandscapePropertyControl::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
	if("lightmap.size" == forKey)
	{
		currentSceneNode->GetCustomProperties()->SetInt32("lightmap.size", newValue);
	}

	NodesPropertyControl::OnIntPropertyChanged(forList, forKey, newValue);
}

void LandscapePropertyControl::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const FilePath &newValue)
{
	Set<String> errorsLog;
	if("property.landscape.heightmap" == forKey)
	{
		bool isValid = SceneValidator::Instance()->ValidateHeightmapPathname(newValue, errorsLog);
		if(isValid)
		{
			Landscape *landscape = GetLandscape();
			if (!landscape)
				return;

			Vector3 size(
				propertyList->GetFloatPropertyValue("property.landscape.size"),
				propertyList->GetFloatPropertyValue("property.landscape.size"),
				propertyList->GetFloatPropertyValue("property.landscape.height"));
			AABBox3 bbox;
			bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
			bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));

			if(!newValue.IsEmpty())
			{
				landscape->BuildLandscapeFromHeightmapImage(newValue, bbox);
			}
		}
	}
	else
	{
		bool isValid = (newValue.IsEmpty()) ? true : SceneValidator::Instance()->ValidateTexturePathname(newValue, errorsLog);
		if(isValid)
		{
            FilePath descriptorPathname;
            if(!newValue.IsEmpty())
            {
                descriptorPathname = TextureDescriptor::GetDescriptorPathname(newValue);
            }
            
			if("property.landscape.texture.tile0" == forKey)
			{
				SetLandscapeTexture(Landscape::TEXTURE_TILE0, descriptorPathname);
			}
			else if("property.landscape.texture.tile1" == forKey)
			{
				SetLandscapeTexture(Landscape::TEXTURE_TILE1, descriptorPathname);
			}
			else if("property.landscape.texture.tile2" == forKey)
			{
				SetLandscapeTexture(Landscape::TEXTURE_TILE2, descriptorPathname);
			}
			else if("property.landscape.texture.tile3" == forKey)
			{
				SetLandscapeTexture(Landscape::TEXTURE_TILE3, descriptorPathname);
			}
			else if("property.landscape.texture.tilemask" == forKey)
			{
				SetLandscapeTexture(Landscape::TEXTURE_TILE_MASK, descriptorPathname);
			}
			else if("property.landscape.texture.color" == forKey)
			{
				SetLandscapeTexture(Landscape::TEXTURE_COLOR, descriptorPathname);
			}
			else if("property.landscape.texture.tiledtexture" == forKey)
			{
				SetLandscapeTexture(Landscape::TEXTURE_TILE_FULL, descriptorPathname);
			}
		}
	}

	if(0 == errorsLog.size())
	{
		NodesPropertyControl::OnFilepathPropertyChanged(forList, forKey, newValue);
	}
	else
	{
		ShowErrorDialog(errorsLog);
	}
}

void LandscapePropertyControl::SetLandscapeTexture(Landscape::eTextureLevel level, const FilePath &texturePathname)
{
	Landscape *landscape = GetLandscape();
	if (!landscape)
		return;

    landscape->SetTexture(level, texturePathname);
    if(!texturePathname.IsEmpty())
    {
        SceneValidator::Instance()->ValidateTextureAndShowErrors(landscape->GetTexture(level), landscape->GetTextureName(level), Format("Landscape. TextureLevel %d", level));
    }

    if(Landscape::TEXTURE_TILE_FULL != level)
    {
        landscape->UpdateFullTiledTexture();   
    }
}



void LandscapePropertyControl::OnComboIndexChanged(
                                PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey)
{
    if("property.landscape.tilemode" == forKey)
    {
		Landscape *landscape = GetLandscape();
		if (!landscape)
			return;

        landscape->SetTiledShaderMode((Landscape::eTiledShaderMode)newItemIndex);
    }
        
    NodesPropertyControl::OnComboIndexChanged(forList, forKey, newItemIndex, newItemKey);
}

void LandscapePropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if("property.landscape.showgrid" == forKey)
    {
        // RETURN TO THIS CODE LATER
//        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
//        
//        if(newValue)
//        {
//            landscape->SetDebugFlags(landscape->GetDebugFlags() | DebugRenderComponent::DEBUG_DRAW_GRID);
//        }
//        else 
//        {
//            landscape->SetDebugFlags(landscape->GetDebugFlags() & ~DebugRenderComponent::DEBUG_DRAW_GRID);
//        }
    }
    else if (String("property.material.fogenabled") == forKey)
    {
		Landscape *landscape = GetLandscape();
		if (!landscape)
			return;

        landscape->SetFog(newValue);
    }

    NodesPropertyControl::OnBoolPropertyChanged(forList, forKey, newValue);
}

void LandscapePropertyControl::OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor)
{
    if("property.material.fogcolor" == forKey)
    {
		Landscape *landscape = GetLandscape();
		if (!landscape)
			return;
        landscape->SetFogColor(newColor);
    }
    
    if("property.landscape.texture.tilecolor0" == forKey)
    {
		Landscape *landscape = GetLandscape();
		if (!landscape)
			return;
        landscape->SetTileColor(Landscape::TEXTURE_TILE0, newColor);
    }
    if("property.landscape.texture.tilecolor1" == forKey)
    {
		Landscape *landscape = GetLandscape();
		if (!landscape)
			return;
        landscape->SetTileColor(Landscape::TEXTURE_TILE1, newColor);
    }
    if("property.landscape.texture.tilecolor2" == forKey)
    {
		Landscape *landscape = GetLandscape();
		if (!landscape)
			return;
        landscape->SetTileColor(Landscape::TEXTURE_TILE2, newColor);
    }
    if("property.landscape.texture.tilecolor3" == forKey)
    {
		Landscape *landscape = GetLandscape();
		if (!landscape)
			return;
        landscape->SetTileColor(Landscape::TEXTURE_TILE3, newColor);
    }
    
    PropertyListDelegate::OnColorPropertyChanged(forList, forKey, newColor);
}


void LandscapePropertyControl::GenerateFullTiledTexture(DAVA::BaseObject *object, void *userData, void *callerData)
{
	Landscape *landscape = GetLandscape();
	if (!landscape)
		return;

    FilePath texPathname = landscape->SaveFullTiledTexture();
    FilePath descriptorPathname = TextureDescriptor::GetDescriptorPathname(texPathname);
    
    TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(descriptorPathname);
    if(!descriptor)
    {
        descriptor = new TextureDescriptor();
        descriptor->pathname = descriptorPathname;
        descriptor->Save();
    }
    
    propertyList->SetFilepathPropertyValue(String("property.landscape.texture.tiledtexture"), descriptor->pathname);
    landscape->SetTexture(Landscape::TEXTURE_TILE_FULL, descriptor->pathname);
    
    SafeRelease(descriptor);
}

void LandscapePropertyControl::SaveHeightmapToPng(DAVA::BaseObject *object, void *userData, void *callerData)
{
	Landscape *landscape = GetLandscape();
	if (!landscape)
		return;

    Heightmap * heightmap = landscape->GetHeightmap();
    FilePath heightmapPath = landscape->GetHeightmapPathname();
    heightmapPath.ReplaceExtension(".png");
    heightmap->SaveToImage(heightmapPath);
}

Landscape* LandscapePropertyControl::GetLandscape() const
{
	RenderComponent* component = cast_if_equal<RenderComponent*>(currentSceneNode->GetComponent(Component::RENDER_COMPONENT));
	Landscape *landscape = dynamic_cast<Landscape*> (component->GetRenderObject());
	DVASSERT(landscape);
	return landscape;
}

