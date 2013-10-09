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



#include "PropertyControlCreator.h"
#include "NodesPropertyControl.h"
#include "LightPropertyControl.h"
#include "CameraPropertyControl.h"
#include "LandscapePropertyControl.h"
#include "LandscapeEditorPropertyControl.h"
#include "MaterialPropertyControl.h"
#include "LodNodePropertyControl.h"
#include "EntityPropertyControl.h"
#include "ParticleEmitterPropertyControl.h"
#include "SwitchNodePropertyControl.h"
#include "ParticleEffectPropertyControl.h"
#include "MeshInstancePropertyControl.h"


PropertyControlCreator::PropertyControlCreator()
{
    for(int32 iControl = 0; iControl < EPCID_COUNT; ++iControl)
    {
        controls[iControl] = NULL;
    }
}

PropertyControlCreator::~PropertyControlCreator()
{
    for(int32 iControl = 0; iControl < EPCID_COUNT; ++iControl)
    {
        SafeRelease(controls[iControl]);
    }
}


NodesPropertyControl * PropertyControlCreator::CreateControlForNode(Entity * sceneNode, const Rect & rect, bool createNodeProperties)
{
	return CreateControlForNode(DetectNodeType(sceneNode), rect, createNodeProperties);
    
}


PropertyControlCreator::ePropertyControlIDs PropertyControlCreator::DetectNodeType(Entity *node)
{
    if(node->GetComponent(Component::LIGHT_COMPONENT))
    {
        return EPCID_LIGHT;
    }
    
    if(node->GetComponent(Component::CAMERA_COMPONENT))
    {
        return EPCID_CAMERA;
    }
    
    if(node->GetComponent(Component::SWITCH_COMPONENT))
    {
        return EPCID_SWITCH;
    }
    
    if(GetEmitter(node))
    {
        return EPCID_PARTICLE_EMITTER;
    }
    
    if(node->GetComponent(Component::PARTICLE_EFFECT_COMPONENT))
    {
        return EPCID_PARTICLE_EFFECT;
    }

    if(node->GetComponent(Component::LOD_COMPONENT))
    {
        return EPCID_LODNODE;
    }

    if(GetLandscape(node))
    {
        return EPCID_LANDSCAPE;
    }
    
    return EPCID_NODE;
}



NodesPropertyControl * PropertyControlCreator::CreateControlForNode(DataNode * dataNode, const Rect & rect, bool createNodeProperties)
{
    Material * material = dynamic_cast<Material *>(dataNode);
	if(material)
	{
        return CreateControlForNode(EPCID_MATERIAL, rect, createNodeProperties);
	}
    
	return CreateControlForNode(EPCID_DATANODE, rect, createNodeProperties);
}

NodesPropertyControl * PropertyControlCreator::CreateControlForNode(
                                                                    ePropertyControlIDs controlID, 
                                                                    const Rect & rect, bool createNodeProperties)
{
    if(controls[controlID] && (rect != controls[controlID]->GetRect()))
    {
        SafeRelease(controls[controlID]);
    }
 
    if(!controls[controlID])
    {
        switch (controlID) 
        {
            case EPCID_LIGHT:
                controls[controlID] = new LightPropertyControl(rect, createNodeProperties);
                break;
            case EPCID_CAMERA:
                controls[controlID] = new CameraPropertyControl(rect, createNodeProperties);
                break;
            case EPCID_LANDSCAPE:
                controls[controlID] = new LandscapePropertyControl(rect, createNodeProperties);
                break;
            case EPCID_MESH:
                controls[controlID] = new MeshInstancePropertyControl(rect, createNodeProperties);
                break;
            case EPCID_NODE:
                controls[controlID] = new NodesPropertyControl(rect, createNodeProperties);
                break;
            case EPCID_LODNODE:
                controls[controlID] = new LodNodePropertyControl(rect, createNodeProperties);
                break;


            case EPCID_LANDSCAPE_EDITOR_MASK:
                controls[controlID] = new LandscapeEditorPropertyControl(rect, createNodeProperties, LandscapeEditorPropertyControl::MASK_EDITOR_MODE);
                break;

            case EPCID_LANDSCAPE_EDITOR_HEIGHT:
                controls[controlID] = new LandscapeEditorPropertyControl(rect, createNodeProperties, LandscapeEditorPropertyControl::HEIGHT_EDITOR_MODE);
                break;

			case EPCID_LANDSCAPE_EDITOR_COLORIZE:
                controls[controlID] = new LandscapeEditorPropertyControl(rect, createNodeProperties, LandscapeEditorPropertyControl::COLORIZE_EDITOR_MODE);
                break;

            case EPCID_DATANODE:
                controls[controlID] = new NodesPropertyControl(rect, createNodeProperties);
                break;
            case EPCID_MATERIAL:
                controls[controlID] = new MaterialPropertyControl(rect, createNodeProperties);
				break;
			case EPCID_PARTICLE_EMITTER:
				controls[controlID] = new ParticleEmitterPropertyControl(rect, createNodeProperties);
				break;
			case EPCID_SWITCH:
				controls[controlID] = new SwitchNodePropertyControl(rect, createNodeProperties);
				break;

			case EPCID_PARTICLE_EFFECT:
				controls[controlID] = new ParticleEffectPropertyControl(rect, createNodeProperties);
				break;

                
            default:
                break; 
        }
    }

    
    return controls[controlID];
}

NodesPropertyControl * PropertyControlCreator::CreateControlForLandscapeEditor(Entity * sceneNode, const Rect & rect, LandscapeEditorPropertyControl::eEditorMode mode)
{
    if(LandscapeEditorPropertyControl::MASK_EDITOR_MODE == mode)
    {
        return CreateControlForNode(EPCID_LANDSCAPE_EDITOR_MASK, rect, false);
    }
    else if(LandscapeEditorPropertyControl::HEIGHT_EDITOR_MODE == mode)
    {
        return CreateControlForNode(EPCID_LANDSCAPE_EDITOR_HEIGHT, rect, false);
    }
	else if(LandscapeEditorPropertyControl::COLORIZE_EDITOR_MODE == mode)
    {
        return CreateControlForNode(EPCID_LANDSCAPE_EDITOR_COLORIZE, rect, false);
    }

    return NULL;
}

NodesPropertyControl * PropertyControlCreator::CreateControlForEntity(Entity * entity, const Rect & rect)
{
	if(controls[EPCID_ENTITY] && (rect != controls[EPCID_ENTITY]->GetRect()))
	{
		SafeRelease(controls[EPCID_ENTITY]);
	}

	if(!controls[EPCID_ENTITY])
	{
		controls[EPCID_ENTITY] = new EntityPropertyControl(rect, false);
	}

	return controls[EPCID_ENTITY];
}
