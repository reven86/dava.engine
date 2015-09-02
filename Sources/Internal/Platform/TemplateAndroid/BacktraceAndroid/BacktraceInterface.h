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
    virtual ~MemoryMapIterator(){}
    virtual bool Next() = 0;
    virtual void ToBegin() = 0;
    virtual const char * GetLib() const = 0;
    virtual pointer_size GetAddrStart() const = 0;
    virtual pointer_size GetAddrEnd() const = 0;
};
//not using Strings, in some places might be unsafe
class MemoryMapInterface
{
public:
    virtual ~MemoryMapInterface(){}
    virtual bool Resolve(pointer_size addr,const char **,pointer_size *) const = 0;

    //! Memory map has ONE preallocated iterator so this function will
    //! always return the same iterator
    virtual MemoryMapIterator & GetIterator() const = 0;

};

class BacktraceInterface
{
public:
    virtual ~BacktraceInterface(){}
    virtual void BuildMemoryMap() = 0;
    virtual const MemoryMapInterface * GetMemoryMap() const = 0 ;
    //handler safe function
    virtual void Backtrace(Function<void (pointer_size)> onFrame, void * context = NULL , void * siginfo = NULL) = 0;
	virtual void PrintableBacktrace(Function<void (pointer_size,const char * str)> onFrame,  void * context = NULL , void * siginfo = NULL) = 0;
};

} /* namespace DAVA */
#endif /* BACKTRACEINTERFACE_H_ */
