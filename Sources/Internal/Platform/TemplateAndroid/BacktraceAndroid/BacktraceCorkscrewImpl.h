#ifndef BACKTRACECORKSCREWIMPL_H_
#define BACKTRACECORKSCREWIMPL_H_

#include "BacktraceInterface.h"
#include "libcorkscrew_stab.h"
namespace DAVA
{
class MemoryMapCorkscrewIterator : public MemoryMapIterator
{
public:
    MemoryMapCorkscrewIterator(map_info_t* map_info);
    virtual ~MemoryMapCorkscrewIterator()
    {
    }
    bool Next() override;
    void ToBegin() override;
    const char* GetLib() const override;
    pointer_size GetAddrStart() const override;
    pointer_size GetAddrEnd() const override;

protected:
    map_info_t* map_info;
    map_info_t* now;
};

class MemoryMapCorkscrewInterface : public MemoryMapInterface
{
public:
    MemoryMapCorkscrewInterface();
    virtual ~MemoryMapCorkscrewInterface();
    bool Resolve(pointer_size addr, const char**, pointer_size*) const override;
    MemoryMapIterator& GetIterator() const override;
    map_info_t* GetMapInfo()
    {
        return map_info;
    }

protected:
    map_info_t* map_info;
    mutable MemoryMapCorkscrewIterator iterator;
};

class BacktraceCorkscrewImpl : public DAVA::BacktraceInterface
{
public:
    virtual ~BacktraceCorkscrewImpl();
    static BacktraceCorkscrewImpl* Load();
    void BuildMemoryMap() override;
    const MemoryMapInterface* GetMemoryMap() const override;
    //handler safe function
    void Backtrace(Function<void(pointer_size)> onFrame,
                   void* context = NULL, void* siginfo = NULL) override;

    void PrintableBacktrace(Function<void(pointer_size, const char* str)> onFrame,
                            void* context = NULL, void* siginfo = NULL) override;

protected:
    BacktraceCorkscrewImpl();
    bool loaded;
    MemoryMapCorkscrewInterface* processMap;
};

} /* namespace DAVA */
#endif /* BACKTRACECORKSCREWIMPL_H_ */
