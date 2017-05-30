#pragma once

#include <Concurrency/Thread.h>
#include <Concurrency/Mutex.h>

#include <Functional/Function.h>

#include <Base/BaseTypes.h>
#include <Logger/Logger.h>

namespace DAVA
{
namespace TArc
{
class SingleThreadStrategy
{
public:
    SingleThreadStrategy();

    void Lock();
    void Unlock();

private:
    Thread::Id threadAffinity;
};

class MultiThreadStrategy
{
public:
    void Lock();
    void Unlock();

private:
    Mutex mutex;
};

template <typename T, typename TLockStrategy>
class ObjectsPool
{
public:
    ObjectsPool(size_t batchSize, size_t initialBatchCount = 1);

    std::shared_ptr<T> RequestObject();

private:
    friend class ObjectsPoolTest;
    static const size_t INVALID_INDEX;

    struct ObjectMemoryTraits
    {
        enum : size_t
        {
            ObjectSize = sizeof(T),
            VoidSize = sizeof(void*),
            Count = static_cast<size_t>((ObjectSize + VoidSize - 1) / static_cast<double>(VoidSize))
        };
    };

    struct ObjectNode
    {
        size_t nodeGeneration;
        size_t nextIndex;
        Array<void*, ObjectMemoryTraits::Count> object;
    };

    struct PoolNode
    {
        PoolNode()
            : batchStart(nullptr)
            , batchHead(nullptr)
        {
        }

        ~PoolNode()
        {
            delete[] batchStart;
        }

        ObjectNode* batchStart;
        ObjectNode* batchHead;
    };

    struct LockGuard
    {
        LockGuard(TLockStrategy& lockStrategy_)
            : lockStrategy(lockStrategy_)
        {
            lockStrategy.Lock();
        }
        ~LockGuard()
        {
            lockStrategy.Unlock();
        }

        TLockStrategy& lockStrategy;
    };

    void ReleaseObject(T* object);
    bool IsOurMemory(T* object);
    PoolNode* AllocateNewBatch();

    const size_t batchSize;
    List<PoolNode> objectBatches;
    TLockStrategy lockStrategy;
};

inline SingleThreadStrategy::SingleThreadStrategy()
    : threadAffinity(Thread::GetCurrentId())
{
}

inline void SingleThreadStrategy::Lock()
{
    DVASSERT(threadAffinity == Thread::GetCurrentId());
}

inline void SingleThreadStrategy::Unlock()
{
    DVASSERT(threadAffinity == Thread::GetCurrentId());
}

inline void MultiThreadStrategy::Lock()
{
    mutex.Lock();
}

inline void MultiThreadStrategy::Unlock()
{
    mutex.Unlock();
}

template <typename T, typename TLockStrategy>
const size_t ObjectsPool<T, TLockStrategy>::INVALID_INDEX = static_cast<size_t>(-1);

template <typename T, typename TLockStrategy>
ObjectsPool<T, TLockStrategy>::ObjectsPool(size_t batchSize_, size_t initialBatchCount)
    : batchSize(batchSize_)
{
    DVASSERT(initialBatchCount > 0);
}

template <typename T, typename TLockStrategy>
std::shared_ptr<T> ObjectsPool<T, TLockStrategy>::RequestObject()
{
    LockGuard guard(lockStrategy);
    PoolNode* poolNode = nullptr;
    for (PoolNode& node : objectBatches)
    {
        if (node.batchHead != nullptr)
        {
            poolNode = &node;
            break;
        }
    }

    if (poolNode == nullptr)
    {
        poolNode = AllocateNewBatch();
        Logger::FrameworkDebug("ObjectsPool: allocate new batch");
    }

    ObjectNode* result = poolNode->batchHead;
    ObjectNode* nextObject = nullptr;
    if (result->nextIndex != INVALID_INDEX)
    {
        DVASSERT(result->nextIndex < batchSize);
        nextObject = poolNode->batchStart + result->nextIndex;
    }
    poolNode->batchHead = nextObject;
    new (result->object.data()) T();

    return std::shared_ptr<T>(reinterpret_cast<T*>(result->object.data()), MakeFunction(this, &ObjectsPool<T, TLockStrategy>::ReleaseObject));
}

template <typename T, typename TLockStrategy>
void ObjectsPool<T, TLockStrategy>::ReleaseObject(T* object)
{
    LockGuard guard(lockStrategy);
    DVASSERT(IsOurMemory(object));
    ObjectNode* objectNode = reinterpret_cast<ObjectNode*>(reinterpret_cast<uint8_t*>(object) - 2 * sizeof(size_t));
    object->~T();
    size_t generation = objectNode->nodeGeneration;
    DVASSERT(generation < objectBatches.size());

    PoolNode& poolNode = *std::next(objectBatches.begin(), generation);
    if (poolNode.batchHead == nullptr)
    {
        objectNode->nextIndex = INVALID_INDEX;
        poolNode.batchHead = objectNode;
    }
    else
    {
        size_t headIndex = poolNode.batchHead - poolNode.batchStart;
        objectNode->nextIndex = headIndex;
        poolNode.batchHead = objectNode;
    }
}

template <typename T, typename TLockStrategy>
typename ObjectsPool<T, TLockStrategy>::PoolNode* ObjectsPool<T, TLockStrategy>::AllocateNewBatch()
{
    size_t generation = objectBatches.size();

    objectBatches.push_back(PoolNode());
    PoolNode& poolNode = objectBatches.back();
    poolNode.batchStart = new ObjectNode[batchSize];
    poolNode.batchHead = poolNode.batchStart + (batchSize - 1);
    for (size_t i = 0; i < batchSize; ++i)
    {
        poolNode.batchStart[i].nodeGeneration = generation;
        poolNode.batchStart[i].nextIndex = i - 1;
    }

    return &poolNode;
}

template <typename T, typename TLockStrategy>
bool ObjectsPool<T, TLockStrategy>::IsOurMemory(T* object)
{
    uint8_t* rawObjectPointer = reinterpret_cast<uint8_t*>(object);
    for (const PoolNode& node : objectBatches)
    {
        uint8_t* endNode = reinterpret_cast<uint8_t*>(node.batchStart + batchSize);
        std::ptrdiff_t diffFromStart = rawObjectPointer - reinterpret_cast<uint8_t*>(node.batchStart);
        std::ptrdiff_t diffFromEnd = endNode - rawObjectPointer;

        if (diffFromStart > 0 && diffFromEnd > 0)
        {
            return true;
        }
    }

    return false;
}
} // namespace TArc
} // namespace DAVA