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


#include "SceneSystemTest.h"

TestSceneSystem::TestSceneSystem(Scene * scene)
: SceneSystem(scene)
{
    
}

TestSceneSystem::~TestSceneSystem()
{
    
}

void TestSceneSystem::RegisterEntity(Entity * entity)
{
    SceneSystem::RegisterEntity(entity);
    unfilteredEntities.insert(entity);
}

void TestSceneSystem::UnregisterEntity(Entity * entity)
{
    unfilteredEntities.erase(entity);
    SceneSystem::UnregisterEntity(entity);
}

void TestSceneSystem::RegisterComponent(Entity * entity, Component * component)
{
    SceneSystem::RegisterComponent(entity, component);
}

void TestSceneSystem::UnregisterComponent(Entity * entity, Component * component)
{
    SceneSystem::UnregisterComponent(entity, component);
}

void TestSceneSystem::AddEntity(Entity * entity)
{
    filteredEntities.insert(entity);
}

void TestSceneSystem::RemoveEntity(Entity * entity)
{
    filteredEntities.erase(entity);
}

SceneSystemTest::SceneSystemTest():
TestTemplate<SceneSystemTest>("SceneSystemTest")
{
	RegisterFunction(this, &SceneSystemTest::RegisterUnregisterAddRemoveTest, "RegisterUnregisterAddRemoveTest", NULL);
}

void SceneSystemTest::LoadResources()
{
    GetBackground()->SetColor(Color::White);
    
    scene = new Scene();
    testSceneSystemAction = new TestSceneSystem(scene);
    scene->AddSystem(testSceneSystemAction, (1 << Component::ACTION_COMPONENT), true);

    testSceneSystemActionSound = new TestSceneSystem(scene);
    scene->AddSystem(testSceneSystemActionSound, (1 << Component::ACTION_COMPONENT) | (1 << Component::SOUND_COMPONENT), true);

    testSceneSystemSound = new TestSceneSystem(scene);
    scene->AddSystem(testSceneSystemSound, (1 << Component::SOUND_COMPONENT), true);
}

void SceneSystemTest::UnloadResources()
{
    SafeRelease(scene);
}


void SceneSystemTest::RegisterUnregisterAddRemoveTest(PerfFuncData * data)
{
    Entity * entity1 = new Entity();
    Entity * entity2 = new Entity();
    Entity * entity3 = new Entity();

    scene->AddNode(entity1);

    TEST_VERIFY(testSceneSystemAction->unfilteredEntities.size() == 1);
    TEST_VERIFY(testSceneSystemAction->filteredEntities.size() == 0);

    TEST_VERIFY(testSceneSystemSound->unfilteredEntities.size() == 1);
    TEST_VERIFY(testSceneSystemSound->filteredEntities.size() == 0);

    TEST_VERIFY(testSceneSystemActionSound->unfilteredEntities.size() == 1);
    TEST_VERIFY(testSceneSystemActionSound->filteredEntities.size() == 0);

    scene->AddNode(entity2);

    TEST_VERIFY(testSceneSystemAction->unfilteredEntities.size() == 2);
    TEST_VERIFY(testSceneSystemAction->filteredEntities.size() == 0);
    TEST_VERIFY(testSceneSystemSound->unfilteredEntities.size() == 2);
    TEST_VERIFY(testSceneSystemSound->filteredEntities.size() == 0);
    TEST_VERIFY(testSceneSystemActionSound->unfilteredEntities.size() == 2);
    TEST_VERIFY(testSceneSystemActionSound->filteredEntities.size() == 0);

    scene->RemoveNode(entity1);

    TEST_VERIFY(testSceneSystemAction->unfilteredEntities.size() == 1);
    TEST_VERIFY(testSceneSystemAction->filteredEntities.size() == 0);
    TEST_VERIFY(testSceneSystemSound->unfilteredEntities.size() == 1);
    TEST_VERIFY(testSceneSystemSound->filteredEntities.size() == 0);
    TEST_VERIFY(testSceneSystemActionSound->unfilteredEntities.size() == 1);
    TEST_VERIFY(testSceneSystemActionSound->filteredEntities.size() == 0);
    
    scene->AddNode(entity1);
    scene->AddNode(entity3);

    TEST_VERIFY(testSceneSystemAction->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemAction->filteredEntities.size() == 0);
    TEST_VERIFY(testSceneSystemSound->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemSound->filteredEntities.size() == 0);
    TEST_VERIFY(testSceneSystemActionSound->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemActionSound->filteredEntities.size() == 0);

    entity1->AddComponent(new SoundComponent());

    TEST_VERIFY(testSceneSystemAction->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemAction->filteredEntities.size() == 0);
    TEST_VERIFY(testSceneSystemSound->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemSound->filteredEntities.size() == 1);
    TEST_VERIFY(testSceneSystemActionSound->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemActionSound->filteredEntities.size() == 0);

    entity1->AddComponent(new ActionComponent());
    
    TEST_VERIFY(testSceneSystemAction->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemAction->filteredEntities.size() == 1);
    TEST_VERIFY(testSceneSystemSound->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemSound->filteredEntities.size() == 1);
    TEST_VERIFY(testSceneSystemActionSound->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemActionSound->filteredEntities.size() == 1);

    entity1->RemoveComponent(Component::SOUND_COMPONENT);
    
    TEST_VERIFY(testSceneSystemAction->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemAction->filteredEntities.size() == 1);
    TEST_VERIFY(testSceneSystemSound->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemSound->filteredEntities.size() == 0);
    TEST_VERIFY(testSceneSystemActionSound->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemActionSound->filteredEntities.size() == 0);

    entity1->RemoveComponent(Component::ACTION_COMPONENT);

    TEST_VERIFY(testSceneSystemAction->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemAction->filteredEntities.size() == 0);
    TEST_VERIFY(testSceneSystemSound->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemSound->filteredEntities.size() == 0);
    TEST_VERIFY(testSceneSystemActionSound->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemActionSound->filteredEntities.size() == 0);
    
    scene->RemoveNode(entity1);
    scene->RemoveNode(entity2);
    scene->RemoveNode(entity3);
    
    TEST_VERIFY(testSceneSystemAction->unfilteredEntities.size() == 0);
    TEST_VERIFY(testSceneSystemAction->filteredEntities.size() == 0);
    TEST_VERIFY(testSceneSystemSound->unfilteredEntities.size() == 0);
    TEST_VERIFY(testSceneSystemSound->filteredEntities.size() == 0);
    TEST_VERIFY(testSceneSystemActionSound->unfilteredEntities.size() == 0);
    TEST_VERIFY(testSceneSystemActionSound->filteredEntities.size() == 0);
    

    entity1->AddComponent(new UpdatableComponent());
    entity1->AddComponent(new ActionComponent());
    entity1->AddComponent(new SoundComponent());

    entity2->AddComponent(new UpdatableComponent());
    entity2->AddComponent(new SoundComponent());

    entity3->AddComponent(new UpdatableComponent());
    entity3->AddComponent(new ActionComponent());
    
    scene->AddNode(entity1);
    scene->AddNode(entity2);
    scene->AddNode(entity3);
    
    TEST_VERIFY(testSceneSystemAction->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemAction->filteredEntities.size() == 2);
    TEST_VERIFY(testSceneSystemSound->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemSound->filteredEntities.size() == 2);
    TEST_VERIFY(testSceneSystemActionSound->unfilteredEntities.size() == 3);
    TEST_VERIFY(testSceneSystemActionSound->filteredEntities.size() == 1);
}

