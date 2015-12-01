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


#ifndef __QUICKED_PACKAGE_ITERATOR_H__
#define __QUICKED_PACKAGE_ITERATOR_H__

#include "Base/BaseTypes.h"
#include "Functional/Function.h"

class PackageNode;
class PackageBaseNode;

class PackageIterator
{
public:
    using MatchFunction = DAVA::Function<bool(const PackageBaseNode*)>;
    PackageIterator(const PackageIterator& it);
    explicit PackageIterator(PackageBaseNode* node, MatchFunction func = defaultFunction);
    ~PackageIterator();
    bool IsValid() const;
    void SetMatchFunction(MatchFunction func);

    PackageIterator& operator=(const PackageIterator& it);

    PackageIterator& operator++();
    const PackageIterator operator++(int);
    PackageIterator& operator+=(int n);

    PackageIterator& operator--();
    const PackageIterator operator--(int);
    PackageIterator& operator-=(int n);

    PackageBaseNode* operator*() const;

private:
    struct IteratorData;
    std::unique_ptr<IteratorData> impl;
    static const MatchFunction defaultFunction;
};

inline const PackageIterator PackageIterator::operator++(int)
{
    PackageIterator it = *this;
    ++(*this);
    return it;
}

inline const PackageIterator PackageIterator::operator--(int)
{
    PackageIterator it = *this;
    --(*this);
    return it;
}

PackageIterator operator+(PackageIterator iter, int n);
PackageIterator operator-(PackageIterator iter, int n);

#endif // __QUICKED_PACKAGE_ITERATOR_H__
