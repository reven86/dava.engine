#ifndef BACKTRACEINTERFACE_H_
#define BACKTRACEINTERFACE_H_
#include "Base/BaseTypes.h"
#include "Functional/Function.h"
namespace DAVA
{
// Can't use heap allocations inside crash handlers,
// so interface is a bit weird because pointer polymorphism
// not fully avaliable
class MemoryMapIterator
{
public:
    virtual ~MemoryMapIterator()
    {
    }
    virtual bool Next() = 0;
    virtual void ToBegin() = 0;
    virtual const char* GetLib() const = 0;
    virtual pointer_size GetAddrStart() const = 0;
    virtual pointer_size GetAddrEnd() const = 0;
};
//not using Strings, in some places might be unsafe
class MemoryMapInterface
{
public:
    virtual ~MemoryMapInterface()
    {
    }
    virtual bool Resolve(pointer_size addr, const char**, pointer_size*) const = 0;

    //! Memory map has ONE preallocated iterator so this function will
    //! always return the same iterator
    virtual MemoryMapIterator& GetIterator() const = 0;
};

class BacktraceInterface
{
public:
    virtual ~BacktraceInterface()
    {
    }
    virtual void BuildMemoryMap() = 0;
    virtual const MemoryMapInterface* GetMemoryMap() const = 0;
    //handler safe function
    virtual void Backtrace(Function<void(pointer_size)> onFrame, void* context = NULL, void* siginfo = NULL) = 0;
    virtual void PrintableBacktrace(Function<void(pointer_size, const char* str)> onFrame, void* context = NULL, void* siginfo = NULL) = 0;
};

} /* namespace DAVA */
#endif /* BACKTRACEINTERFACE_H_ */
