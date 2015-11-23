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


#ifndef __CLIPTEST_TEST_H__
#define __CLIPTEST_TEST_H__

#include "DAVAEngine.h"
#include "Infrastructure/BaseScreen.h"

using namespace DAVA;

class ClipTest : public BaseScreen
{
public:
    ClipTest();

protected:
    void LoadResources() override;
    void UnloadResources() override;

private:
    void MoveDown(BaseObject* obj, void* data, void* callerData);
    void MoveUp(BaseObject* obj, void* data, void* callerData);
    void MoveRight(BaseObject* obj, void* data, void* callerData);
    void MoveLeft(BaseObject* obj, void* data, void* callerData);
    void StartPos(BaseObject* obj, void* data, void* callerData);
    void DebugDrawPressed(BaseObject* obj, void* data, void* callerData);
    void ClipPressed(BaseObject* obj, void* data, void* callerData);

    bool enableClip = true;
    bool enableDebugDraw = true;
    Rect defaultRect;

    UIControl* fullSizeWgt;
    UIControl* parent1;
    UIControl* child1;
    UIControl* parent2;
    UIControl* child2;

    UIButton* clip;
    UIButton* debugDraw;
    UIButton* startPos;
    UIButton* moveLeft;
    UIButton* moveRight;
    UIButton* moveUp;
    UIButton* moveDown;
};

#endif //__CLIPTEST_TEST_H__
