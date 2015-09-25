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


#ifndef __DAVAENGINE_DATA_H__
#define __DAVAENGINE_DATA_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"

namespace DAVA
{

/** 
	\ingroup baseobjects
	\brief class to store static or dynamic byte array
*/

	
class Data : public BaseObject
{
protected:
	~Data();
public:
    Data(uint32 _size);
    Data(uint8 * _data, uint32 _size);
    
    /**
        \brief Get pointer to data array
        \returns pointer
     */
    inline const uint8* GetPtr() const;
    inline uint8 * GetPtr();
    /**
        \brief Get size of this date object
        \returns size
     */
    inline uint32 GetSize() const;

private:
    uint8 * data;
    uint32 size;
};
    
// Implementation
inline uint8* Data::GetPtr()
{
    return data;
}

inline const uint8* Data::GetPtr() const
{
    return data;
}

inline uint32 Data::GetSize() const
{
    return size;
}

}; 


#endif // __DAVAENGINE_DATA_H__

