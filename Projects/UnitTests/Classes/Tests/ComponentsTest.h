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



#ifndef __COMPONENTS_TEST_H__
#define __COMPONENTS_TEST_H__

#include "DAVAEngine.h"
using namespace DAVA;

#include "TestTemplate.h"

class SingleComponentSystem: public SceneSystem
{
public:
    SingleComponentSystem(Scene * scene);

    virtual void AddEntity(Entity * entity);
    virtual void RemoveEntity(Entity * entity);
    
	virtual void AddComponent(Entity * entity, Component * component);
	virtual void RemoveComponent(Entity * entity, Component * component);

    uint32 GetEnititesCount() const;
    uint32 GetComponentsCount() const;
    
    
    Vector<Component *> components;
    Vector<Entity *>entities;
};


class MultiComponentSystem: public SceneSystem
{
public:
    MultiComponentSystem(Scene * scene);
    
    virtual void AddEntity(Entity * entity);
    virtual void RemoveEntity(Entity * entity);
    
	virtual void AddComponent(Entity * entity, Component * component);
	virtual void RemoveComponent(Entity * entity, Component * component);
    
    uint32 GetEnititesCount() const;
    uint32 GetComponentsCount(uint32 componentType) const;

    
    Map<uint32, Vector<Component *> > components;
    Vector<Entity *>entities;
};



class ComponentsTest : public TestTemplate<ComponentsTest>
{
protected:
    ~ComponentsTest(){}
public:
	ComponentsTest();

	virtual void LoadResources();
	virtual void UnloadResources();

	void RegisterEntityTest(PerfFuncData * data);
    
	void AddComponentTest1(PerfFuncData * data);
	void AddComponentTest2(PerfFuncData * data);
	void AddComponentTest3(PerfFuncData * data);

    void MultiComponentTest1(PerfFuncData * data);
    void MultiComponentTest2(PerfFuncData * data);
    void MultiComponentTest3(PerfFuncData * data);
};


#endif // __COMPONENTS_TEST_H__
