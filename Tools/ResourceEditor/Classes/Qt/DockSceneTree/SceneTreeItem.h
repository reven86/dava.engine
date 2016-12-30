#pragma once

#include <QStandardItem>

// framework
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleForce.h"

#include "Classes/Selection/Selectable.h"

Q_DECLARE_METATYPE(DAVA::Entity*);
Q_DECLARE_METATYPE(DAVA::ParticleLayer*);
Q_DECLARE_METATYPE(DAVA::ParticleForce*);
Q_DECLARE_METATYPE(DAVA::ParticleEmitterInstance*);

class SceneTreeModel;
class SceneTreeFilteringModel;

class SceneTreeItem : public QStandardItem
{
public:
    enum eItemType : DAVA::uint32
    {
        EIT_Entity,
        EIT_Emitter,
        EIT_Layer,
        EIT_Force,
        EIT_InnerEmitter
    };

    enum eItemDataRole : DAVA::uint32
    {
        EIDR_Type = Qt::UserRole,
        EIDR_Data,
    };

    SceneTreeItem(eItemType type, DAVA::BaseObject* object_);
    virtual ~SceneTreeItem() = default;

    QVariant data(int role) const;

    DAVA::uint32 ItemType() const;
    virtual const QIcon& ItemIcon() const;

    virtual QString ItemName() const = 0;
    virtual QVariant ItemData() const = 0;

    bool IsAcceptedByFilter() const;
    void SetAcceptByFilter(bool state);
    bool IsHighlighed() const;
    void SetHighlight(bool state);

    DAVA::BaseObject* GetItemObject() const;

protected:
    Selectable object;
    eItemType type = EIT_Entity;
    bool isAcceptedByFilter = false;
    bool isHighlighted = false;
};

class SceneTreeItemEntity : public SceneTreeItem
{
public:
    static DAVA::Entity* GetEntity(SceneTreeItem* item);
    static void DoSync(QStandardItem* rootItem, DAVA::Entity* entity);

public:
    SceneTreeItemEntity(DAVA::Entity* entity);

    DAVA::Entity* GetEntity() const;

    QString ItemName() const override;
    QVariant ItemData() const override;
    const QIcon& ItemIcon() const override;
};

class SceneTreeItemParticleEmitter : public SceneTreeItem
{
public:
    static DAVA::ParticleEmitterInstance* GetEmitterInstance(SceneTreeItem* item);
    static DAVA::ParticleEmitterInstance* GetEmitterInstanceStrict(SceneTreeItem* item);

    static void DoSync(QStandardItem* rootItem, DAVA::ParticleEmitterInstance* layer);

public:
    SceneTreeItemParticleEmitter(DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitterInstance* instance);

    DAVA::ParticleEmitterInstance* GetEmitterInstance() const;

    QString ItemName() const override;
    QVariant ItemData() const override;
    const QIcon& ItemIcon() const override;

    DAVA::ParticleEffectComponent* effect = nullptr;
};

class SceneTreeItemParticleLayer : public SceneTreeItem
{
public:
    static DAVA::ParticleLayer* GetLayer(SceneTreeItem* item);
    static void DoSync(QStandardItem* rootItem, DAVA::ParticleLayer* layer);

public:
    SceneTreeItemParticleLayer(DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitterInstance* instance, DAVA::ParticleLayer* layer);

    DAVA::ParticleLayer* GetLayer() const;

    QString ItemName() const override;
    QVariant ItemData() const override;
    const QIcon& ItemIcon() const override;

    DAVA::ParticleEffectComponent* effect = nullptr;
    DAVA::ParticleEmitterInstance* emitterInstance = nullptr;
    bool hasInnerEmmiter = false;
};

class SceneTreeItemParticleForce : public SceneTreeItem
{
public:
    static DAVA::ParticleForce* GetForce(SceneTreeItem* rootItem);

public:
    SceneTreeItemParticleForce(DAVA::ParticleLayer* layer, DAVA::ParticleForce* force);

    DAVA::ParticleForce* GetForce() const;

    QString ItemName() const override;
    QVariant ItemData() const override;
    const QIcon& ItemIcon() const override;

    DAVA::ParticleLayer* layer = nullptr;
};

class SceneTreeItemParticleInnerEmitter : public SceneTreeItemParticleEmitter
{
public:
    SceneTreeItemParticleInnerEmitter(DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* parentLayer);

    QString ItemName() const override;

    DAVA::ParticleLayer* parent = nullptr;

private:
    DAVA::ScopedPtr<DAVA::ParticleEmitterInstance> localInstance;
};
