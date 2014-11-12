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

#ifndef __DAVAENGINE_JOB_H__
#define __DAVAENGINE_JOB_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/Message.h"
#include "Platform/Thread.h"

namespace DAVA
{


class Job : public BaseObject
{
public:
	enum eState
	{
		STATUS_UNDONE,
		STATUS_DONE
	};

	enum ePerformedWhere
	{
		PERFORMED_ON_CREATOR_THREAD,
		PERFORMED_ON_MAIN_THREAD
	};

    enum eCreationFlags
    {
        NO_FLAGS = 0,
        RETAIN_WHILE_NOT_COMPLETED = 1 << 0, //<! job will retain underlying BaseObject if one is found in Message, and release when job is done
    };

    static const uint32 DEFAULT_FLAGS = RETAIN_WHILE_NOT_COMPLETED;

	Job(const Message & message, const Thread::Id & creatorThreadId, uint32 flags);	eState GetState();
	ePerformedWhere PerformedWhere();
    const Message & GetMessage();

    uint32 GetFlags() const;

protected:
	void Perform();
	void SetState(eState newState);
	void SetPerformedOn(ePerformedWhere performedWhere);

	Message message;
	Thread::Id creatorThreadId;

	eState state;
	ePerformedWhere performedWhere;

    uint32 flags;

	friend class MainThreadJobQueue;
	friend class JobManager;
};

inline 
const Message & Job::GetMessage()
{
    return message;
}

inline uint32 Job::GetFlags() const
{
    return flags;
}

}

#endif //__DAVAENGINE_JOB_H__