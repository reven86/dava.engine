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



#ifndef __ResourceEditorQt__BaseParticleEditorNode__
#define __ResourceEditorQt__BaseParticleEditorNode__

#include "Scene3D/ParticleEffectNode.h"
#include "DockSceneGraph/ExtraUserData.h"

namespace DAVA {

class ParticleEffectComponent;
	
// Base Particle Editor node - common for all inner nodes.
class BaseParticleEditorNode : public ExtraUserData
{
public:
    BaseParticleEditorNode(Entity* rootNode);
    virtual ~BaseParticleEditorNode();
    
    // Add/remove child node.
	virtual void AddNode(BaseParticleEditorNode* childNode);
    virtual void InsertBeforeNode(BaseParticleEditorNode* childNode, BaseParticleEditorNode* beforeNode);
	
	void RemoveNode(BaseParticleEditorNode* childNode, bool needDeleteNode = true);

	// Change the order of the children.
	void MoveNode(BaseParticleEditorNode* childNode, BaseParticleEditorNode* beforeNode);

    // Access to the root node.
    Entity* GetRootNode() const {return rootNode;};

	// Access to the parent node.
	BaseParticleEditorNode* GetParentNode() const {return parentNode;};

	// Access to the Particle Effect Component.
	ParticleEffectComponent* GetParticleEffectComponent() const;

    // Node name.
    void SetNodeName(const QString& nodeName) {this->nodeName = nodeName;};
    virtual QString GetName() const {return this->nodeName;};

    // Mark for selection logic.
    bool IsMarkedForSelection() const {return this->isMarkedForSelection;};
    void SetMarkedToSelection(bool value) {this->isMarkedForSelection = value;};

    // Access to children.
    typedef List<BaseParticleEditorNode*> PARTICLEEDITORNODESLIST;
    const PARTICLEEDITORNODESLIST& GetChildren() const {return childNodes;};

	KeyedArchive* GetExtraData();
	void ClearExtraData();

protected:
	// Set the parent of the node.
	void SetParentNode(BaseParticleEditorNode* parentNode) {this->parentNode = parentNode; };

    // Cleanup the node's children.
    void Cleanup();

    // Root effect node.
    Entity* rootNode;

	// Our parent.
	BaseParticleEditorNode* parentNode;

    // Current node name.
    QString nodeName;

    // Child nodes.
    PARTICLEEDITORNODESLIST childNodes;
    
    // Whether the node is marked for selection.
    bool isMarkedForSelection;

	KeyedArchive* extraData;
};

}

#endif /* defined(__ResourceEditorQt__BaseParticleEditorNode__) */
