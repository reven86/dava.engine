#ifndef __DAVAENGINE_THREAD_H__
#define __DAVAENGINE_THREAD_H__ 

#include <functional>

#include "Base/BaseTypes.h"
#include "Base/Message.h"
#include "Base/BaseObject.h"
#include "Concurrency/Atomic.h"
#include "Concurrency/ConcurrentObject.h"
#include "Concurrency/Mutex.h"

#if !defined(__DAVAENGINE_WINDOWS__)
#include <pthread.h>
#endif

namespace DAVA
{
/**
	\defgroup threads Thread wrappers
*/

#if defined(__DAVAENGINE_WINDOWS__)

class ThreadTraits
{
public:
    using Id = DWORD;

protected:
    using Handle = HANDLE;
};

#else

class ThreadTraits
{
public:
    using Id = pthread_t;

protected:
    using Handle = pthread_t;

#ifdef __DAVAENGINE_ANDROID__
    pid_t system_handle = 0;
#endif
};

#endif

class Thread : public ThreadTraits, public BaseObject
{
#if defined(__DAVAENGINE_WINDOWS__)
    friend unsigned __stdcall ThreadFunc(void* param);
#else
    friend void* PthreadMain(void* param);
#endif
public:
    using Procedure = Function<void()>;

    enum eThreadPriority
    {
        PRIORITY_LOW = 0,
        PRIORITY_NORMAL,
        PRIORITY_HIGH
    };

    enum eThreadState
    {
        STATE_CREATED = 0,
        STATE_RUNNING,
        STATE_ENDED,
        STATE_KILLED
    };

    /**
    \brief main thread name
    */
    static const char* davaMainThreadName;

    /**
		\brief static function to detect if current thread is main thread of application
		\returns true if now main thread executes
	*/
    static bool IsMainThread();

    /**
		\brief static function to create instance of thread object based on Message.
		This functions create thread based on message. 
        It do not start the thread until Start function called.
		\returns ptr to thread object 
	*/
    static Thread* Create(const Message& msg);

    /**
        \brief static function to create instance of thread object based on Procedure.
        This functions create thread based on function with signature 'void()'.
        It do not start the thread until Start function called.
        \returns ptr to thread object
     */
    static Thread* Create(const Procedure& proc);

    /**
     \brief Sets thread name. You should to use it before Thread::Start().
     */
    inline void SetName(const String& _name);
    inline const String& GetName() const;

    /**
		\brief Start execution of the thread
	*/
    void Start();

    /**
		\brief get current thread state. 

		This function return state of the thread. It can be STATE_CREATED, STATE_RUNNING, STATE_ENDED.
	*/
    inline eThreadState GetState() const;

    void SetPriority(eThreadPriority priority);
    inline eThreadPriority GetPriority() const;

    /** Wait until thread's finished.
    */
    void Join();
    bool IsJoinable() const;

    /** Kill thread by OS. No signals will be sent.
    */
    void Kill();
    static void KillAll();

    /** Ask to cancel thred. User should to check state variable
    */
    inline void Cancel();
    /** Check if someone asked thiead to cancel
     */
    inline bool IsCancelling() const;
    static void CancelAll();

    /**
     \brief Notifies the scheduler that the current thread is
     willing to release its processor to other threads of the same or higher
     priority.
     */
    static void Yield();

    /**
     \brief suspends the execution of the current thread until the time-out interval elapses
     */
    static void Sleep(uint32 timeMS);

    /**
     \brief 
     \returns returns unique current thread identifier.
    */
    static Id GetCurrentId();

    /**
     \brief 
     \returns returns current thread identifier as integer
    */
    static uint64 GetCurrentIdAsUInt64();

    /**
     \returns returns Id of Thread Object.
     */
    inline Id GetId() const;

    /**
    \brief sets stack size of the thread. Stack size cannot be set to running thread
    */
    inline void SetStackSize(size_t size);

    /**
     \brief register current native thread handle and remember it's Id as Id of MainThread.
     */
    static void InitMainThread();

    /**
    \brief sets name of current thread
    */
    static void SetCurrentThreadName(const String& str);

    /**
    \brief bind current thread to specified processor. Thread cannot be run on other processors.
    */
    bool BindToProcessor(unsigned proc_n);

private:
    Thread();
    Thread(const Message& msg);
    Thread(const Procedure& proc);
    virtual ~Thread();

    void Init();
    void Shutdown();

    /**
    \brief Kill thread native implementation (contains no Thread logic)
    */
    void KillNative();

    /**
    \brief Function which processes in separate thread. Used to launch user defined code and handle state.
    */
    static void ThreadFunction(void* param);

    Procedure threadFunc;
    Atomic<eThreadState> state;
    Atomic<bool> isCancelling;
    Atomic<bool> isJoinable{ false };
    size_t stackSize;
    eThreadPriority threadPriority;

    /**
    \brief Native thread handle - variable which used to thread manipulations
    */
    Handle handle;
    /**
    \brief Some value which is unique for any thread in current OS. Could be used only for equals comparision.
    */
    Id id;

    /**
    \brief Name of the thread.
    */
    String name;

    /**
    \brief Full list of created DAVA::Thread's. Main thread is not DAVA::Thread, so it is not there.
    */
    static Id mainThreadId;
    static Id glThreadId;
};

inline void Thread::SetName(const String& _name)
{
    // name sets inside thread function, so we cannot change thread name after starting.
    DVASSERT(STATE_CREATED == state);
    name = _name;
}

inline const String& Thread::GetName() const
{
    return name;
}

inline Thread::eThreadState Thread::GetState() const
{
    return state.Get();
}

inline void Thread::Cancel()
{
    isCancelling = true;
}

inline bool Thread::IsJoinable() const
{
    return isJoinable.Get();
}

inline bool Thread::IsCancelling() const
{
    return isCancelling.Get();
}

inline Thread::Id Thread::GetId() const
{
    return id;
}

inline void Thread::SetStackSize(size_t size)
{
    DVASSERT(STATE_CREATED == state);
    stackSize = size;
}

inline Thread::eThreadPriority Thread::GetPriority() const
{
    return threadPriority;
}
};

#endif // __DAVAENGINE_THREAD_H__
