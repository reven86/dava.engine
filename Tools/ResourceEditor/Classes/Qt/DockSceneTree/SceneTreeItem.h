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


#ifndef __SCENE_TREE_ITEM_H__
#define __SCENE_TREE_ITEM_H__

#include <QStandardItem>

// framework
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleForce.h"

Q_DECLARE_METATYPE(DAVA::Entity*);
Q_DECLARE_METATYPE(DAVA::ParticleLayer*);
Q_DECLARE_METATYPE(DAVA::ParticleForce*);
Q_DECLARE_METATYPE(DAVA::ParticleEmitter*);


class SceneTreeModel;
class SceneTreeFilteringModel;

class SceneTreeItem
    : public QStandardItem
{
public:
	enum eItemType
	{
		EIT_Entity,
		EIT_Emitter,
		EIT_Layer,
		EIT_Force, 
		EIT_InnerEmitter
	};

	enum eItemDataRole
	{
		EIDR_Type = Qt::UserRole,
		EIDR_Data,
	};

	SceneTreeItem(eItemType type);
	~SceneTreeItem();

	QVariant data(int role) const;

	int ItemType() const;
	virtual QIcon ItemIcon() const;

	virtual QString ItemName() const = 0;
	virtual QVariant ItemData() const = 0;

    bool IsAcceptedByFilter() const;
    void SetAcceptByFilter(bool state);
    bool IsHighlighed() const;
    void SetHighlight(bool state);

protected:
	eItemType type;
	QIcon iconCache;
    bool isAcceptedByFilter;
    bool isHighlighted;
};

class SceneTreeItemEntity : public SceneTreeItem
{
public:
	SceneTreeItemEntity(DAVA::Entity* entity);
	~SceneTreeItemEntity();

	static DAVA::Entity* GetEntity(SceneTreeItem *item);
	static void DoSync(QStandardItem *rootItem, DAVA::Entity *entity);

	virtual QString ItemName() const;
	virtual QVariant ItemData() const;
	virtual QIcon ItemIcon() const;    

	DAVA::Entity *entity;
};

class SceneTreeItemParticleEmitter : public SceneTreeItem
{
public:
	SceneTreeItemParticleEmitter(DAVA::ParticleEffectComponent *effect, DAVA::ParticleEmitter* emitter);
	~SceneTreeItemParticleEmitter();

	static DAVA::ParticleEmitter* GetEmitter(SceneTreeItem *item);
    static DAVA::ParticleEmitter* GetEmitterStrict(SceneTreeItem *item);
	static void DoSync(QStandardItem *rootItem, DAVA::ParticleEmitter *layer);

	virtual QString ItemName() const;
	virtual QVariant ItemData() const;
	virtual QIcon ItemIcon() const;
	//virtual QVariant ItemBackgroundColor() const;

	DAVA::ParticleEffectComponent *effect;
	DAVA::ParticleEmitter *emitter;
	
};

class SceneTreeItemParticleLayer : public SceneTreeItem
{
public:
	SceneTreeItemParticleLayer(DAVA::ParticleEffectComponent *effect, DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer *layer);
	~SceneTreeItemParticleLayer();

	static DAVA::ParticleLayer* GetLayer(SceneTreeItem *item);	
	static void DoSync(QStandardItem *rootItem, DAVA::ParticleLayer *layer);

	virtual QString ItemName() const;
	virtual QVariant ItemData() const;
	virtual QIcon ItemIcon() const;

	DAVA::ParticleEffectComponent *effect;
	DAVA::ParticleEmitter *emitter;
	DAVA::ParticleLayer *layer;
	bool hasInnerEmmiter;
};

class SceneTreeItemParticleForce : public SceneTreeItem
{
public:
	SceneTreeItemParticleForce(DAVA::ParticleLayer *layer, DAVA::ParticleForce *force);
	~SceneTreeItemParticleForce();

	static DAVA::ParticleForce* GetForce(SceneTreeItem *rootItem);

	virtual QString ItemName() const;
	virtual QVariant ItemData() const;
	virtual QIcon ItemIcon() const;

	DAVA::ParticleLayer *layer;
	DAVA::ParticleForce *force;
};

class SceneTreeItemParticleInnerEmitter : public SceneTreeItemParticleEmitter
{
public:
	SceneTreeItemParticleInnerEmitter(DAVA::ParticleEffectComponent *effect, DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer *parentLayer);
	~SceneTreeItemParticleInnerEmitter();		

	virtual QString ItemName() const;	
	
	DAVA::ParticleLayer *parent;	
};


#endif // __QT_PROPERTY_ITEM_H__
