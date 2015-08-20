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

#ifndef __UNIVERSAL_TEST_H__
#define __UNIVERSAL_TEST_H__

#include "Tests/BaseTest.h"
#include "Tests/Utils/WaypointsInterpolator.h"
#include "Tests/Utils/TankUtils.h"

class UniversalTest : public BaseTest
{
public:
    
    UniversalTest(const TestParams& params);
    
    static const String TEST_NAME;
    
protected:
    
    void LoadResources() override;
    void PerformTestLogic(float32 timeElapsed) override;
    
private:
    
    static const FastName CAMERA;
    
    static const FastName CAMERA_PATH;
    static const FastName TANK_STUB;
    static const FastName TANKS;
    
    static const float32 TANK_ROTATION_ANGLE;
    
    Map<FastName, std::pair<Entity*, Vector<uint16>>> skinnedTankData;
    List<Entity*> tankStubs;
    
    std::unique_ptr<WaypointsInterpolator> waypointInterpolator;
    
    ScopedPtr<Camera> camera;
    Vector3 camPos;
    Vector3 camDst;
    
    float32 time;
};

#endif 
