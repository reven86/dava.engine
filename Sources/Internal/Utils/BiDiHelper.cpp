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


#include "BiDiHelper.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/LockGuard.h"
#include "Utils/StringUtils.h"

#include "fribidi/fribidi.h"
#include "fribidi/fribidi-bidi-types.h"
#include "fribidi/fribidi-unicode.h"

namespace DAVA
{

/** \brief A FriBiDi library wrapper. */
class BiDiWrapper
{
public:
    bool Prepare(const WideString& logicalStr, WideString& preparedStr, bool* isRTL);
    bool Reorder(const WideString& preparedStr, WideString& reorderedStr, const bool forceRtl);
private:
    Mutex mutex;
    Vector<FriBidiChar> logicalBuffer;
    Vector<FriBidiChar> visualBuffer;
    Vector<FriBidiCharType> bidiTypes;
    Vector<FriBidiLevel> bidiLevels;
    Vector<FriBidiArabicProp> arabicProps;
    Vector<char8> charBuffer;
};

bool BiDiWrapper::Prepare(WideString const& logicalStr, WideString& preparedStr, bool* isRTL)
{
    LockGuard<Mutex> guard(mutex);

    static FriBidiFlags flags = FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC;

    FriBidiParType base_dir = FRIBIDI_PAR_ON;
    uint32 fribidi_len = static_cast<uint32>(logicalStr.length());

    logicalBuffer.assign(logicalStr.begin(), logicalStr.end());
    bidiTypes.resize(fribidi_len);
    bidiLevels.resize(fribidi_len);
    visualBuffer.resize(fribidi_len);
    arabicProps.resize(fribidi_len);

    fribidi_get_bidi_types(&logicalBuffer[0], fribidi_len, &bidiTypes[0]);

    FriBidiLevel max_level = fribidi_get_par_embedding_levels(&bidiTypes[0], fribidi_len, &base_dir, &bidiLevels[0]) - 1;
    if (max_level < 0)
    {
        return false;
    }

    Memcpy(&visualBuffer[0], &logicalBuffer[0], fribidi_len * sizeof(FriBidiChar));

    /* Arabic joining */
    fribidi_get_joining_types(&logicalBuffer[0], fribidi_len, &arabicProps[0]);
    fribidi_join_arabic(&bidiTypes[0], fribidi_len, &bidiLevels[0], &arabicProps[0]);
    fribidi_shape(flags, &bidiLevels[0], fribidi_len, &arabicProps[0], &visualBuffer[0]);

    /* Remove FRIBIDI_CHAR_FILL aka 'ZERO WIDTH NO-BREAK SPACE' (U+FEFF) after joining and shaping */
    auto lastIt = std::remove_if(visualBuffer.begin(), visualBuffer.end(), [](FriBidiChar ch){ return ch == FRIBIDI_CHAR_FILL; });
    visualBuffer.erase(lastIt, visualBuffer.end());

    preparedStr.assign(visualBuffer.begin(), visualBuffer.end());
    if (isRTL)
    {
        *isRTL = FRIBIDI_IS_RTL(base_dir);
    }
    
    return true;
}

bool BiDiWrapper::Reorder(const WideString& preparedStr, WideString& reorderedStr, bool const forceRtl)
{
    LockGuard<Mutex> guard(mutex);

    static FriBidiFlags flags = FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC;

    FriBidiParType base_dir = forceRtl ? (FRIBIDI_PAR_RTL) : (FRIBIDI_PAR_ON);
    uint32 fribidi_len = static_cast<uint32>(preparedStr.length());

    logicalBuffer.assign(preparedStr.begin(), preparedStr.end());
    bidiTypes.resize(fribidi_len);
    bidiLevels.resize(fribidi_len);
    visualBuffer.resize(fribidi_len);

    fribidi_get_bidi_types(&logicalBuffer[0], fribidi_len, &bidiTypes[0]);

    FriBidiLevel max_level = fribidi_get_par_embedding_levels(&bidiTypes[0], fribidi_len, &base_dir, &bidiLevels[0]) - 1;
    if (max_level < 0)
    {
        return false;
    }

    Memcpy(&visualBuffer[0], &logicalBuffer[0], fribidi_len * sizeof(FriBidiChar));

    fribidi_boolean status = fribidi_reorder_line(flags, &bidiTypes[0], fribidi_len, 0, base_dir, &bidiLevels[0], &visualBuffer[0], NULL);
    if (status == 0)
    {
        return false;
    }

    fribidi_len = fribidi_remove_bidi_marks(&visualBuffer[0], fribidi_len, NULL, NULL, NULL);

    reorderedStr.assign(&visualBuffer[0], &visualBuffer[0] + fribidi_len);

    return true;
}

BiDiHelper::BiDiHelper()
    : wrapper(new BiDiWrapper())
{
}

BiDiHelper::~BiDiHelper()
{
    SAFE_DELETE(wrapper);
}

bool BiDiHelper::PrepareString(const WideString& logicalStr, WideString& preparedStr, bool* isRTL) const
{
    return wrapper->Prepare(logicalStr, preparedStr, isRTL);
}

bool BiDiHelper::ReorderString(const WideString& preparedStr, WideString& reorderedStr, const bool forceRtl) const
{
    return wrapper->Reorder(preparedStr, reorderedStr, forceRtl);
}

bool BiDiHelper::IsBiDiSpecialCharacter(uint32 character) const
{
    return FRIBIDI_IS_EXPLICIT_OR_BN(fribidi_get_bidi_type(character))
        || character == FRIBIDI_CHAR_LRM 
        || character == FRIBIDI_CHAR_RLM;
}

}
