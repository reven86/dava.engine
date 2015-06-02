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


#ifndef BACKTRACECORKSCREWIMPL_H_
#define BACKTRACECORKSCREWIMPL_H_

#include "BacktraceInterface.h"
#include "libcorkscrew_stab.h"
namespace DAVA
{
class MemoryMapCorkscrewIterator:public MemoryMapIterator
{
public:
    MemoryMapCorkscrewIterator(map_info_t *map_info);
    virtual ~MemoryMapCorkscrewIterator(){}
    bool Next() override;
    void ToBegin() override;
    const char * GetLib() const override;
    pointer_size GetAddrStart() const override;
    pointer_size GetAddrEnd() const override;
protected:
    map_info_t *map_info;
    map_info_t *now;
};

class MemoryMapCorkscrewInterface: public MemoryMapInterface
{
public:
    MemoryMapCorkscrewInterface();
    virtual ~MemoryMapCorkscrewInterface();
    bool Resolve(pointer_size addr,const char **,pointer_size *) const override;
    MemoryMapIterator & GetIterator() const override;
    map_info_t* GetMapInfo(){return map_info;}
protected:

    map_info_t *map_info;
    mutable MemoryMapCorkscrewIterator iterator;
};

class BacktraceCorkscrewImpl: public DAVA::BacktraceInterface
{
public:

    virtual ~BacktraceCorkscrewImpl();
    static  BacktraceCorkscrewImpl* Load();
    void BuildMemoryMap() override;
    const MemoryMapInterface * GetMemoryMap() const override;
    //handler safe function
    void Backtrace(Function<void(pointer_size)> onFrame,
            void * context = NULL, void * siginfo = NULL) override;
            
    void PrintableBacktrace(Function<void (pointer_size,const char * str)> onFrame, 
            void * context = NULL , void * siginfo = NULL) override;
protected:
    BacktraceCorkscrewImpl();
    bool loaded;
    MemoryMapCorkscrewInterface * processMap;
};

} /* namespace DAVA */
#endif /* BACKTRACECORKSCREWIMPL_H_ */
