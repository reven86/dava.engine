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

#ifndef __MATERIALS_TEST_H__
#define __MATERIALS_TEST_H__

#include "BaseTest.h"

class MaterialsTest : public BaseTest
{
public:
    MaterialsTest(const TestParams& testParams);
    ~MaterialsTest();
    
    void BeginFrame() override;
    
    bool IsFinished() const override;
    
    static const String TEST_NAME;
    
protected:
  
    void LoadResources() override;
    void PerformTestLogic(float32 timeElapsed) override {};
    
    void PrintStatistic(const Vector<BaseTest::FrameInfo>& frames) override;
    
    const String& GetSceneName() const override;
    
private:
    
    Entity* CreateSpeedTreeEntity(Entity* entity);
    Entity* CreateSkinnedEntity(Entity* entity);
    Entity* CreateEntityForLightmapMaterial(Entity* entity);
    
    void ReplacePlanes(const Vector<Entity*>& planes);
    
    static const String SPHERICAL_LIT_MATERIAL;
    static const String SKINNED_MATERIAL;
    static const String LIGHTMAP_MATERIAL;
    
    static const FastName LIGHT_ENTITY;
    static const FastName CAMERA_ENTITY;
    static const FastName PLANE_ENTITY;
    static const FastName MATERIALS_ENTITY;
    
    static const uint32 FRAMES_PER_MATERIAL_TEST;
    
    int32  currentTestStartFrame;
    uint64 currentTestStartTime;
    uint32 currentMaterialIndex;
    
    Vector<Entity*> planes;
    Vector<Entity*> spoPlanes;
    Vector<Entity*> skinnedPlanes;
    Vector<Entity*> lightmapMaterialPlanes;
    
    Vector<NMaterial*> materials;
    Vector<float32> materialTestsElapsedTime;
};

#endif
