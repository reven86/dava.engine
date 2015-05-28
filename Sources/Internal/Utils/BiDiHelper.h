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


#ifndef __DAVAENGINE_BIDIHELPER_H__
#define __DAVAENGINE_BIDIHELPER_H__

#include "Base/BaseTypes.h"

namespace DAVA
{

class BiDiWrapper;

class BiDiHelper
{
public:
    BiDiHelper();
    virtual ~BiDiHelper();

    /**
    * \brief Prepare string for BiDi transformation (shape arabic string). Need for correct splitting.
    * \param [in] logicalStr The logical string.
    * \param [out] preparedStr The prepared string.
    * \param [out] isRTL If non-null, store in isRTL true if line contains Right-to-left text.
    * \return true if it succeeds, false if it fails.
    */
    bool PrepareString(const WideString& logicalStr, WideString& preparedStr, bool* isRTL) const;

    /**
    * \brief Reorder characters in string based.
    * \param [in,out] string The string.
    * \param forceRtl (Optional) true if input text is mixed and must be processed as RTL.
    * \return true if it succeeds, false if it fails.
    */
    bool ReorderString(WideString& string, const bool forceRtl = false) const;

    bool IsBiDiSpecialCharacter(uint32 character) const;

private:
    BiDiWrapper* wrapper;
};

}

#endif