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

#ifndef __QTTOOLS_NGTCOLLECTIONSIMPL_H__
#define __QTTOOLS_NGTCOLLECTIONSIMPL_H__

#include "Base/BaseTypes.h"

#include "core_variant/collection.hpp"

namespace DAVA
{
class InspColl;
class KeyedArchive;
struct MetaInfo;
}

namespace NGTLayer
{
class NGTCollection : public CollectionImplBase
{
    class Iterator;

public:
    NGTCollection(void* object, const DAVA::InspColl* collectionImpl);

    bool empty() const;
    size_t size() const override;

    CollectionIteratorImplPtr begin() override;
    CollectionIteratorImplPtr end() override;

    std::pair<CollectionIteratorImplPtr, bool> get(const Variant& key, GetPolicy policy) override;

    CollectionIteratorImplPtr erase(const CollectionIteratorImplPtr& pos) override;
    CollectionIteratorImplPtr erase(const CollectionIteratorImplPtr& first, const CollectionIteratorImplPtr& last) override;
    size_t erase(const Variant& key) override;

    const TypeId& keyType() const override;
    const TypeId& valueType() const override;
    const TypeId& containerType() const override;
    const void* container() const override;
    int flags() const override;

private:
    void* object;
    const DAVA::InspColl* collectionImpl;
    TypeId keyId;
    TypeId valueId;
    TypeId containerId;
};

class NGTKeyedArchiveImpl : public CollectionImplBase
{
    class Iterator;

public:
    NGTKeyedArchiveImpl(DAVA::KeyedArchive* keyedArchive);

    bool empty() const;
    size_t size() const override;

    CollectionIteratorImplPtr begin() override;
    CollectionIteratorImplPtr end() override;
    std::pair<CollectionIteratorImplPtr, bool> get(const Variant& key, GetPolicy policy) override;
    CollectionIteratorImplPtr erase(const CollectionIteratorImplPtr& pos) override;
    size_t erase(const Variant& key) override;
    CollectionIteratorImplPtr erase(const CollectionIteratorImplPtr& first, const CollectionIteratorImplPtr& last) override;

    const TypeId& keyType() const override;
    const TypeId& valueType() const override;
    const TypeId& containerType() const override;
    const void* container() const override;
    int flags() const override;

private:
    DAVA::KeyedArchive* archive;
    TypeId keyTypeId;
    TypeId valueTypeId;
    TypeId containerTypeId;
};

} // namespace NGTLayer

#endif // __QTTOOLS_NGTCOLLECTIONSIMPL_H__