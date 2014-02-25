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



#ifndef __PROPERTY_CONTROL_CREATOR_H__
#define __PROPERTY_CONTROL_CREATOR_H__

#include "DAVAEngine.h"
#include "LandscapeEditorPropertyControl.h"

using namespace DAVA;

class NodesPropertyControl;
class PropertyControlCreator: public Singleton<PropertyControlCreator>
{
    enum ePropertyControlIDs
    {
        EPCID_LIGHT,
        EPCID_CAMERA,
        EPCID_LANDSCAPE,
        EPCID_MESH,
        EPCID_LODNODE,
        EPCID_NODE,
        
        EPCID_DATANODE,
        EPCID_MATERIAL,
        
        EPCID_LANDSCAPE_EDITOR_MASK,
        EPCID_LANDSCAPE_EDITOR_HEIGHT,
		EPCID_LANDSCAPE_EDITOR_COLORIZE,

		EPCID_ENTITY,
		EPCID_PARTICLE_EMITTER,

		EPCID_SWITCH,

		EPCID_PARTICLE_EFFECT,
        
        EPCID_COUNT
    };
    
public:
	
    PropertyControlCreator();
    virtual ~PropertyControlCreator();
    
    NodesPropertyControl * CreateControlForLandscapeEditor(Entity * sceneNode, const Rect & rect, LandscapeEditorPropertyControl::eEditorMode mode);
    NodesPropertyControl * CreateControlForNode(Entity * sceneNode, const Rect & rect, bool createNodeProperties);
	NodesPropertyControl * CreateControlForNode(DataNode * dataNode, const Rect & rect, bool createNodeProperties);
	NodesPropertyControl * CreateControlForEntity(Entity * entity, const Rect & rect);


private:
    
    NodesPropertyControl * CreateControlForNode(ePropertyControlIDs controlID, const Rect & rect, bool createNodeProperties);

    
    NodesPropertyControl *controls[EPCID_COUNT];
    
    ePropertyControlIDs DetectNodeType(Entity *node);
    
};

#endif //__PROPERTY_CONTROL_CREATOR_H__
