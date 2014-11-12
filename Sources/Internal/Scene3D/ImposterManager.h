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



#ifndef __DAVAENGINE_IMPOSTER_MANAGER_H__
#define __DAVAENGINE_IMPOSTER_MANAGER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/Observer.h"
#include "Scene3D/ImposterNode.h"

namespace DAVA
{

class SharedFBO;
class Scene;

class ImposterManager : public BaseObject
{
protected:
	virtual ~ImposterManager();
public:
	static const int32 MAX_UPDATES_PER_FRAME = 3;

	ImposterManager(Scene * scene);

	bool IsEmpty();
	void Add(ImposterNode * node);
	void Remove(ImposterNode * node);

	void Update(float32 frameTime);
	void Draw();

	void AddToQueue(ImposterNode * node);
	void RemoveFromQueue(ImposterNode * node);
	void UpdateQueue(ImposterNode * node);
    void ProcessQueue();

	void CreateFBO();
	void ReleaseFBO();
	SharedFBO * GetFBO();

private:
	List<ImposterNode*> imposters;
	List<ImposterNode*> queue;

	void AddToPrioritizedPosition(ImposterNode * node);

	SharedFBO * sharedFBO;
	Scene * scene;
};

};

#endif //__DAVAENGINE_IMPOSTER_MANAGER_H__
