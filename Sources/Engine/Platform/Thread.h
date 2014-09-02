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


#ifndef __DAVAENGINE_THREAD_H__
#define __DAVAENGINE_THREAD_H__ 

#include "Base/BaseTypes.h"
#include "Base/Message.h"
#include "Base/BaseObject.h"
#include "Mutex.h"

#if defined (__DAVAENGINE_WIN32__)
#include "Platform/TemplateWin32/pThreadWin32.h"
#else
#include <pthread.h>
#include <signal.h>
#endif

namespace DAVA
{
/**
	\defgroup threads Thread wrappers
*/

/**
	\ingroup threads
	\brief wrapper class to give us level of abstraction on thread implementation in particual OS. Now is supports Win32, MacOS, iPhone platforms.
*/

class ConditionalVariable
{
public:
    ConditionalVariable();
    ~ConditionalVariable();

private:
    pthread_cond_t cv;
    pthread_mutex_t exMutex;

    friend class Thread;
};

class Thread : public BaseObject
{
public:
	enum eThreadState
	{
		STATE_CREATED = 0,
		STATE_RUNNING,
		STATE_ENDED,
        STATE_KILLED
	};

	class ThreadId
	{
#if defined (__DAVAENGINE_WIN32__)
	public:
		bool operator<(const ThreadId & otherThread) const 
		{ 
			return internalTid < otherThread.internalTid; 
		}

	private:
		ThreadId(DWORD _internalTid = 0) 
		{ 
			internalTid = _internalTid; 
		}
		bool operator==(const ThreadId & otherThread) const
		{ 
			return internalTid == otherThread.internalTid; 
		}
		DWORD internalTid;
#else
    public:
		bool operator<(const ThreadId & otherThread) const
		{
			return internalTid < otherThread.internalTid;
		}
        
    private:
        ThreadId(pthread_t _internalTid = 0)
		{
			internalTid = _internalTid;
		}
		bool operator==(const ThreadId & otherThread) const
		{
			return internalTid == otherThread.internalTid;
		}
		pthread_t internalTid;
#endif
		friend class Thread;
	};
	
	/**
		\brief static function to detect if current thread is main thread of application
		\returns true if now main thread executes
	*/
	static	bool IsMainThread();

	/**
		\brief static function to create instance of thread object based on Message.

		This function create thread based on message. It do not start the thread until Start function called.

		\returns ptr to thread object 
	*/
	static Thread		* Create(const Message& msg);

    static void KillAll();

	/**
		\brief start execution of the thread
	*/
	void			Start();
	/**
		\brief get current thread state. 

		This function return state of the thread. It can be STATE_CREATED, STATE_RUNNING, STATE_ENDED.
	*/
	eThreadState	GetState();

    /** Wait until thread's finished.
    */
    void Join();

    /** Kill thread
    */
    void Kill();

    /**
        Wrapp pthread wait, signal and broadcast
	*/
    static void Wait(ConditionalVariable * cv);
    static void Signal(ConditionalVariable * cv);
    static void Broadcast(ConditionalVariable * cv);
    
    /**
     \brief Notifies the scheduler that the current thread is
     willing to release its processor to other threads of the same or higher
     priority.
     */
    static void YieldThread();

    /**
     \brief suspends the execution of the current thread until the time-out interval elapses
     */
    static void SleepThread(uint32 timeMS);

	static ThreadId GetCurrentThreadId();

	void SetThreadId(const ThreadId & threadId);
	ThreadId GetThreadId();

private:
    ~Thread();
	Thread(const Message& msg);
	
	Message	msg;
	eThreadState state;

	ThreadId threadId;
	static ThreadId mainThreadId;

    static Set<Thread *> threadList;
    static Mutex threadListMutex;
	
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
	#if defined (__DAVAENGINE_NPAPI__)
		CGLContextObj npAPIGLContext;
	#endif // #if defined (__DAVAENGINE_NPAPI__)

	friend void	* PthreadMain(void * param);
	void		StartMacOS();
	static void	InitMacOS();
#elif defined (__DAVAENGINE_WIN32__)
public:
	static HDC		currentDC;
	static HGLRC	secondaryContext;

private:
    HANDLE threadHandle;
	void		StartWin32();
	friend DWORD WINAPI ThreadFunc(void* param);
public:
	static void	InitMainThread();

#elif defined(__DAVAENGINE_ANDROID__)
private:
	static ThreadId glThreadId;
private:
	friend void	* PthreadMain(void * param);
	void StartAndroid();
public:
	static void	InitMainThread();
	static void	InitGLThread();
#else //PLATFORMS
	// other platforms
#endif //PLATFORMS	
};

};

#endif // __DAVAENGINE_THREAD_H__
