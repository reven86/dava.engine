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



#ifndef __POINTER_HOLDER_H__
#define __POINTER_HOLDER_H__

#include <QObject>
#include <QVariant>
#include <QMetaType>
#include <QDataStream>

#include "DAVAEngine.h"

//To Use this class need to DECLARE_POINTER_TYPE() for you pointer class and register new type via RegisterPointerType() function


#define DECLARE_POINTER_TYPE(type) \
    Q_DECLARE_METATYPE(PointerHolder<type>); \
    Q_DECLARE_METATYPE(QList<PointerHolder<type> >) \


void RegisterBasePointerTypes();


template<class PointerType>
class PointerHolder
{
public:
    
    PointerHolder();
    PointerHolder(const PointerHolder &fromHolder);
    virtual ~PointerHolder();
    
    static QVariant ToQVariant(PointerType item);
    static PointerType ToPointer(const QVariant &variant);
    
    void SetPointer(PointerType pointer);
    PointerType GetPointer() const;
    
    PointerType storedPointer;
};


template<class PointerType>
QDataStream& operator<<(QDataStream& ostream, const PointerHolder<PointerType>& ph)
{
    const PointerType item = ph.GetPointer();
    
    uint sizeOfPointer = sizeof(item);
    ostream.writeRawData((const char *)&item, sizeOfPointer);
    
    return ostream;
}

template<class PointerType>
QDataStream& operator>>(QDataStream& istream, PointerHolder<PointerType>& ph)
{
    PointerType item = NULL;
    uint sizeOfPointer = sizeof(item);
    istream.readRawData((char *)&item, sizeOfPointer);
    
    ph.SetPointer(item);
    return istream;
}


template <class PointerType>
void RegisterPointerType(const DAVA::String &typeName)
{
    qRegisterMetaTypeStreamOperators<PointerHolder<PointerType> >(DAVA::Format("PointerHolder< %s >", typeName.c_str()));
    qRegisterMetaTypeStreamOperators<QList<PointerHolder<PointerType> > >(DAVA::Format("QList<PointerHolder< %s > >", typeName.c_str()));
}

template<class S>
struct Traits {
    template <class T>
    static int method() { return 0; };
};


template<class PointerType>
PointerHolder<PointerType>::PointerHolder()
{
    storedPointer = NULL;
}

template<class PointerType>
PointerHolder<PointerType>::PointerHolder(const PointerHolder<PointerType> &fromHolder)
{
    storedPointer = fromHolder.storedPointer;
}

template<class PointerType>
PointerHolder<PointerType>::~PointerHolder()
{
    storedPointer = NULL;
}

template<class PointerType>
void PointerHolder<PointerType>::SetPointer(PointerType pointer)
{
    storedPointer = pointer;
}

template<class PointerType>
PointerType PointerHolder<PointerType>::GetPointer() const
{
    return storedPointer;
}


template<class PointerType>
QVariant PointerHolder<PointerType>::ToQVariant(PointerType item)
{
    PointerHolder<PointerType> holder;
    holder.SetPointer(item);
    
    QVariant variant = QVariant::fromValue(holder);
    return variant;
}

template<class PointerType>
PointerType PointerHolder<PointerType>::ToPointer(const QVariant &variant)
{
    PointerHolder<PointerType> holder = variant.value< PointerHolder<PointerType> >();
    return holder.GetPointer();
}



#endif // __POINTER_HOLDER_H__
