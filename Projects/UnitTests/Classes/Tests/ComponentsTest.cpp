#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"
#include "Scene3D/Lod/LodComponent.h"

using namespace DAVA;

class SingleComponentSystem : public SceneSystem
{
public:
    SingleComponentSystem(Scene* scene);

    virtual void AddEntity(Entity* entity);
    virtual void RemoveEntity(Entity* entity);

    virtual void AddComponent(Entity* entity, Component* component);
    virtual void RemoveComponent(Entity* entity, Component* component);

    uint32 GetEnititesCount() const;
    uint32 GetComponentsCount() const;

    Vector<Component*> components;
    Vector<Entity*> entities;
};

class MultiComponentSystem : public SceneSystem
{
public:
    MultiComponentSystem(Scene* scene);

    virtual void AddEntity(Entity* entity);
    virtual void RemoveEntity(Entity* entity);

    virtual void AddComponent(Entity* entity, Component* component);
    virtual void RemoveComponent(Entity* entity, Component* component);

    uint32 GetEnititesCount() const;
    uint32 GetComponentsCount(uint32 componentType) const;

    Map<uint32, Vector<Component*>> components;
    Vector<Entity*> entities;
};

template <class T>
void RemovePointerFromVector(DAVA::Vector<T*>& elements, const T* element)
{
    size_t size = elements.size();
    for (size_t index = 0; index < size; ++index)
    {
        if (element == elements[index])
        {
            elements[index] = elements[size - 1];
            elements.pop_back();
            return;
        }
    }
    DVASSERT(0);
}

SingleComponentSystem::SingleComponentSystem(Scene* scene)
    : SceneSystem(scene)
{
}

void SingleComponentSystem::AddEntity(Entity* entity)
{
    entities.push_back(entity);
    for (int32 id = 0; id < Component::COMPONENT_COUNT; ++id)
    {
        uint64 flags = 1LL << id;
        if ((flags & GetRequiredComponents()) == flags)
        {
            uint32 componentsCount = entity->GetComponentCount(id);
            for (uint32 c = 0; c < componentsCount; ++c)
            {
                AddComponent(entity, entity->GetComponent(id, c));
            }
        }
    }
}

void SingleComponentSystem::RemoveEntity(Entity* entity)
{
    for (int32 id = 0; id < Component::COMPONENT_COUNT; ++id)
    {
        uint64 flags = 1LL << id;
        if ((flags & GetRequiredComponents()) == flags)
        {
            uint32 componentsCount = entity->GetComponentCount(id);
            for (uint32 c = 0; c < componentsCount; ++c)
            {
                RemoveComponent(entity, entity->GetComponent(id, c));
            }
        }
    }
    RemovePointerFromVector(entities, entity);
}

void SingleComponentSystem::AddComponent(Entity* entity, Component* component)
{
    components.push_back(component);
}

void SingleComponentSystem::RemoveComponent(Entity* entity, Component* component)
{
    RemovePointerFromVector(components, component);
}

uint32 SingleComponentSystem::GetEnititesCount() const
{
    return static_cast<uint32>(entities.size());
}

uint32 SingleComponentSystem::GetComponentsCount() const
{
    return static_cast<uint32>(components.size());
}

//=============================================
MultiComponentSystem::MultiComponentSystem(Scene* scene)
    : SceneSystem(scene)
{
}

void MultiComponentSystem::AddEntity(Entity* entity)
{
    entities.push_back(entity);

    for (int32 id = 0; id < Component::COMPONENT_COUNT; ++id)
    {
        uint64 flags = 1LL << id;
        if ((flags & GetRequiredComponents()) == flags)
        {
            uint32 componentsCount = entity->GetComponentCount(id);
            for (uint32 c = 0; c < componentsCount; ++c)
            {
                AddComponent(entity, entity->GetComponent(id, c));
            }
        }
    }
}

void MultiComponentSystem::RemoveEntity(Entity* entity)
{
    for (int32 id = 0; id < Component::COMPONENT_COUNT; ++id)
    {
        uint64 flags = 1LL << id;
        if ((flags & GetRequiredComponents()) == flags)
        {
            uint32 componentsCount = entity->GetComponentCount(id);
            for (uint32 c = 0; c < componentsCount; ++c)
            {
                RemoveComponent(entity, entity->GetComponent(id, c));
            }
        }
    }
    RemovePointerFromVector(entities, entity);
}

void MultiComponentSystem::AddComponent(Entity* entity, Component* component)
{
    components[component->GetType()].push_back(component);
}

void MultiComponentSystem::RemoveComponent(Entity* entity, Component* component)
{
    RemovePointerFromVector(components[component->GetType()], component);
}

uint32 MultiComponentSystem::GetEnititesCount() const
{
    return static_cast<uint32>(entities.size());
}

uint32 MultiComponentSystem::GetComponentsCount(uint32 componentType) const
{
    auto found = components.find(componentType);
    if (found != components.end())
    {
        return static_cast<uint32>(found->second.size());
    }

    return 0;
}

DAVA_TESTCLASS (ComponentsTest)
{
    DAVA_TEST (RegisterEntityTest)
    {
        Scene* scene = new Scene();
        SingleComponentSystem* testSystemLight = new SingleComponentSystem(scene);
        SingleComponentSystem* testSystemAction = new SingleComponentSystem(scene);
        scene->AddSystem(testSystemLight, 1 << Component::LIGHT_COMPONENT);
        scene->AddSystem(testSystemAction, 1 << Component::ACTION_COMPONENT);

        Entity* e1 = new Entity();
        e1->AddComponent(new LightComponent());
        e1->AddComponent(new ActionComponent());

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 0);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 0);

        scene->AddNode(e1);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 1);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 1);

        scene->RemoveNode(e1);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 0);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 0);

        e1->Release();
        scene->Release();
    }

    DAVA_TEST (AddComponentTest1)
    {
        Scene* scene = new Scene();
        SingleComponentSystem* testSystemLight = new SingleComponentSystem(scene);
        SingleComponentSystem* testSystemAction = new SingleComponentSystem(scene);
        scene->AddSystem(testSystemLight, 1 << Component::LIGHT_COMPONENT);
        scene->AddSystem(testSystemAction, 1 << Component::ACTION_COMPONENT);

        Entity* e1 = new Entity();
        scene->AddNode(e1);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 0);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 0);

        e1->AddComponent(new ActionComponent());
        e1->AddComponent(new LightComponent());

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 1);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 1);

        e1->AddComponent(new ActionComponent());
        e1->AddComponent(new LightComponent());

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 2);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 2);

        e1->RemoveComponent(Component::ACTION_COMPONENT);
        e1->RemoveComponent(Component::LIGHT_COMPONENT);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 1);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 1);

        e1->AddComponent(new ActionComponent());
        e1->AddComponent(new LightComponent());

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 2);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 2);

        scene->RemoveNode(e1);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 0);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 0);

        e1->Release();
        scene->Release();
    }

    DAVA_TEST (AddComponentTest2)
    {
        Scene* scene = new Scene();
        SingleComponentSystem* testSystemLight = new SingleComponentSystem(scene);
        SingleComponentSystem* testSystemAction = new SingleComponentSystem(scene);
        scene->AddSystem(testSystemLight, 1 << Component::LIGHT_COMPONENT);
        scene->AddSystem(testSystemAction, 1 << Component::ACTION_COMPONENT);

        Entity* e1 = new Entity();
        Component* a = new ActionComponent();
        Component* l = new LightComponent();

        scene->AddNode(e1);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 0);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 0);

        e1->AddComponent(a);
        e1->AddComponent(l);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 1);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 1);

        e1->RemoveComponent(a);
        e1->RemoveComponent(l);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 0);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 0);

        a = l = NULL;
        scene->RemoveNode(e1);

        e1->Release();
        scene->Release();
    }

    DAVA_TEST (AddComponentTest3)
    {
        Scene* scene = new Scene();
        SingleComponentSystem* testSystemLight = new SingleComponentSystem(scene);
        SingleComponentSystem* testSystemAction = new SingleComponentSystem(scene);
        scene->AddSystem(testSystemLight, 1 << Component::LIGHT_COMPONENT);
        scene->AddSystem(testSystemAction, 1 << Component::ACTION_COMPONENT);

        Entity* e1 = new Entity();
        e1->AddComponent(new ActionComponent());
        e1->AddComponent(new LodComponent());
        e1->AddComponent(new LightComponent());
        e1->AddComponent(new LightComponent());

        Entity* e2 = new Entity();

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 0);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 0);

        scene->AddNode(e1);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 1);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 2);

        e1->AddComponent(new ActionComponent());
        e1->AddComponent(new LightComponent());

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 2);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 3);

        scene->RemoveNode(e1);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 0);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 0);

        scene->AddNode(e1);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 2);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 3);

        scene->AddNode(e2);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 2);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 3);

        e2->AddComponent(new LodComponent());

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 2);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 3);

        e2->AddComponent(new ActionComponent());

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 2);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 3);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 3);

        e2->AddComponent(new LightComponent());

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 2);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 3);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 2);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 4);

        e1->RemoveComponent(Component::ACTION_COMPONENT);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 2);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 2);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 2);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 4);

        e1->RemoveComponent(Component::LIGHT_COMPONENT);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 2);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 2);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 2);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 3);

        e1->AddComponent(new ActionComponent());
        e1->AddComponent(new LightComponent());

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 2);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 3);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 2);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 4);

        scene->RemoveNode(e1);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 1);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 1);

        scene->RemoveNode(e2);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 0);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 0);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 0);

        Entity* e3 = e1->Clone();

        scene->AddNode(e1);
        TEST_VERIFY(testSystemAction->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 2);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 1);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 3);

        scene->AddNode(e3);

        TEST_VERIFY(testSystemAction->GetEnititesCount() == 2);
        TEST_VERIFY(testSystemAction->GetComponentsCount() == 4);
        TEST_VERIFY(testSystemLight->GetEnititesCount() == 2);
        TEST_VERIFY(testSystemLight->GetComponentsCount() == 6);

        e2->Release();
        e1->Release();
        scene->Release();
    }

    DAVA_TEST (MultiComponentTest1)
    {
        Scene* scene = new Scene();
        MultiComponentSystem* testSystem = new MultiComponentSystem(scene);
        scene->AddSystem(testSystem, (1 << Component::LIGHT_COMPONENT) | (1 << Component::ACTION_COMPONENT));

        Entity* e1 = new Entity();
        Component* a = new ActionComponent();
        Component* l = new LightComponent();

        scene->AddNode(e1);

        TEST_VERIFY(testSystem->GetEnititesCount() == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 0);

        e1->AddComponent(a);

        TEST_VERIFY(testSystem->GetEnititesCount() == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 0);

        e1->AddComponent(l);

        TEST_VERIFY(testSystem->GetEnititesCount() == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 1);

        e1->AddComponent(new ActionComponent());

        TEST_VERIFY(testSystem->GetEnititesCount() == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 2);

        e1->RemoveComponent(a);

        TEST_VERIFY(testSystem->GetEnititesCount() == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 1);

        e1->RemoveComponent(l);

        TEST_VERIFY(testSystem->GetEnititesCount() == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 0);

        a = l = NULL;

        scene->RemoveNode(e1);

        e1->Release();
        scene->Release();
    }

    DAVA_TEST (MultiComponentTest2)
    {
        Scene* scene = new Scene();
        MultiComponentSystem* testSystem = new MultiComponentSystem(scene);
        scene->AddSystem(testSystem, (1 << Component::LIGHT_COMPONENT) | (1 << Component::ACTION_COMPONENT));

        Entity* e1 = new Entity();
        Component* a = new ActionComponent();
        Component* l = new LightComponent();

        scene->AddNode(e1);

        TEST_VERIFY(testSystem->GetEnititesCount() == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 0);

        e1->AddComponent(a);

        TEST_VERIFY(testSystem->GetEnititesCount() == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 0);

        e1->AddComponent(l);

        TEST_VERIFY(testSystem->GetEnititesCount() == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 1);

        e1->RemoveComponent(a);

        TEST_VERIFY(testSystem->GetEnititesCount() == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 0);

        e1->RemoveComponent(l);

        TEST_VERIFY(testSystem->GetEnititesCount() == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 0);

        a = l = NULL;

        scene->RemoveNode(e1);

        e1->Release();
        scene->Release();
    }

    DAVA_TEST (MultiComponentTest3)
    {
        Scene* scene = new Scene();
        MultiComponentSystem* testSystem = new MultiComponentSystem(scene);
        scene->AddSystem(testSystem, (1 << Component::LIGHT_COMPONENT) | (1 << Component::ACTION_COMPONENT));

        Entity* e1 = new Entity();

        scene->AddNode(e1);

        TEST_VERIFY(testSystem->GetEnititesCount() == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 0);

        e1->AddComponent(new ActionComponent());

        TEST_VERIFY(testSystem->GetEnititesCount() == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 0);

        e1->AddComponent(new LightComponent());

        TEST_VERIFY(testSystem->GetEnititesCount() == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 1);

        e1->AddComponent(new ActionComponent());

        TEST_VERIFY(testSystem->GetEnititesCount() == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 2);

        e1->RemoveComponent(Component::ACTION_COMPONENT);

        TEST_VERIFY(testSystem->GetEnititesCount() == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 1);

        e1->RemoveComponent(Component::LIGHT_COMPONENT);

        TEST_VERIFY(testSystem->GetEnititesCount() == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 0);

        e1->AddComponent(new ActionComponent());

        TEST_VERIFY(testSystem->GetEnititesCount() == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 0);

        e1->AddComponent(new LightComponent());

        TEST_VERIFY(testSystem->GetEnititesCount() == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 2);

        e1->AddComponent(new ActionComponent());

        TEST_VERIFY(testSystem->GetEnititesCount() == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 3);

        e1->AddComponent(new ActionComponent());

        TEST_VERIFY(testSystem->GetEnititesCount() == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 4);

        Entity* e2 = new Entity();
        e2->AddComponent(new ActionComponent());
        e2->AddComponent(new ActionComponent());
        e2->AddComponent(new LightComponent());
        e2->AddComponent(new LightComponent());

        scene->AddNode(e2);

        TEST_VERIFY(testSystem->GetEnititesCount() == 2);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 3);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 6);

        e2->AddComponent(new ActionComponent());
        e2->AddComponent(new LightComponent());

        TEST_VERIFY(testSystem->GetEnititesCount() == 2);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 4);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 7);

        e2->AddComponent(new LodComponent());

        TEST_VERIFY(testSystem->GetEnititesCount() == 2);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 4);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 7);

        scene->RemoveNode(e1);

        TEST_VERIFY(testSystem->GetEnititesCount() == 1);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 3);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 3);

        scene->RemoveNode(e2);

        TEST_VERIFY(testSystem->GetEnititesCount() == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::LIGHT_COMPONENT) == 0);
        TEST_VERIFY(testSystem->GetComponentsCount(Component::ACTION_COMPONENT) == 0);

        e2->Release();
        e1->Release();
        scene->Release();
    }
}
;
