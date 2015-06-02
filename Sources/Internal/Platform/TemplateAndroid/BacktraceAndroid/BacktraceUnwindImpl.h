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


#ifndef BACKTRACEUNWINDIMPL_H_
#define BACKTRACEUNWINDIMPL_H_

#include "BacktraceInterface.h"
#include "Base/BaseTypes.h"
#include "libunwind_stab.h"
#if defined(__arm__)
namespace DAVA
{
class MemoryMapUnwindIterator:public MemoryMapIterator
{
public:
    MemoryMapUnwindIterator(Vector<unw_map_t>::const_iterator begin,Vector<unw_map_t>::const_iterator end);
    virtual ~MemoryMapUnwindIterator(){}
    bool Next() override;
    void ToBegin() override;
    const char * GetLib() const override;
    pointer_size GetAddrStart() const override;
    pointer_size GetAddrEnd() const override;
protected:
    Vector<unw_map_t>::const_iterator begin;
    Vector<unw_map_t>::const_iterator end;
    Vector<unw_map_t>::const_iterator now;
};

class MemoryMapUnwind: public MemoryMapInterface
{
public:
    MemoryMapUnwind();
    virtual ~MemoryMapUnwind();
    bool Resolve(pointer_size addr,const char **,pointer_size *) const override;
    MemoryMapIterator & GetIterator() const override;

private:

    //vector is ok should be loaded long before crash
    Vector<unw_map_t> memoryMap;
    unw_map_cursor_t mapCursor;
    mutable MemoryMapUnwindIterator iterator;
};
class BacktraceUnwindImpl:public BacktraceInterface
{
public:
    virtual ~BacktraceUnwindImpl();
    static BacktraceUnwindImpl * Load();
    void BuildMemoryMap() override;
    const MemoryMapInterface * GetMemoryMap() const override;
    //handler safe function
    void Backtrace(Function<void(pointer_size)> onFrame, void * context = nullptr , void * siginfo = nullptr) override;
    void PrintableBacktrace(Function<void (pointer_size,const char * str)> onFrame,  void * context = nullptr , void * siginfo = nullptr) override;
protected:
    BacktraceUnwindImpl();
    void BacktraceInternal(Function<void(pointer_size)> onFrame,
                Function<void (pointer_size,const char * str)> onFrameName, 
                void * context = NULL , void * siginfo = NULL);
    void PreBacktrace();
    bool loaded;
    MemoryMapUnwind * processMap;

};

} /* namespace DAVA */
#endif /* __arm__*/
#endif /* BACKTRACEUNWINDIMPL_H_ */
