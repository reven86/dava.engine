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

#ifndef __DAVAENGINE_TEXT_LAYOT_H__
#define __DAVAENGINE_TEXT_LAYOT_H__

#include "Base/BaseTypes.h"
#include "Render/2D/Font.h"
#include "Utils/BiDiHelper.h"

namespace DAVA
{

/**
 * \brief TextLayout class
 * \details Using for splitting text to lines with specified width
 */
class TextLayout
{
public:
    /**
     * \brief Create TextLayout with word wrap and disabled BiDi transformations
     */
    TextLayout();

    /**
     * \brief Create TextLayout with specified wrap mode and BiDi transformations
     * \param[in] useBiDi true for enabling BiDi transformations
     */
    TextLayout(const bool useBiDi);

    /**
     * \brief Destructor
     */
    virtual ~TextLayout();

    /**
    * \brief Set process text for text splitting
    * \param[in] input text to process
    */
    void Reset(const WideString& input);

    /**
    * \brief Set process text and font for text splitting
    * \param[in] input text to process
    * \param[in] font font for detecting characters sizes
    */
    DAVA_DEPRECATED(void Reset(const WideString& input, const Font& font));

    /**
     * \brief Set sizes for characters of text
     * \param[in] charSizes characters sizes
     */
    void SetCharSizes(const Vector<float32>& charSizes);

    /**
    * \brief Calculate sizes for characters of text from font
    * \param[in] font specified font
    */
    void CalculateCharSizes(const Font& font);

    /**
     * \brief Puts cursor to given position
     * \param[in] position cursor position in input text
     */
    void Seek(const uint32 position);

    /**
     * \brief Returns current cursor position
     * \return current cursor position
     */
    const uint32 Tell() const;
    
    /**
     * \brief Checks that text didn't finished yet
     * \return false if not all text processed
     */
    bool IsEndOfText();

    /**
     * \brief Split text by words from current cursor position with specified width
     * \param[in] lineWidth maximum line width in pixels
     * \return true if text can be split by words
     */
    bool NextByWords(const float32 lineWidth);
    
    /**
     * \brief Split text by symbols from current cursor position with specified width
     * \param[in] lineWidth maximum line width in pixels
     * \return always return true and set in current line minimum one symbol
     */
    bool NextBySymbols(const float32 lineWidth);

    /**
     * \brief Checks that input text is Right-To-Left
     * \return true if input text is Right-To-Left text or has Right-To-Left blocks
     */
    const bool IsRtlText() const;

    /**
     * \brief Returns original text
     * \return original text that sets with TextLayout::Reset
     */
    const WideString& GetOriginalText() const;

    /**
     * \brief Returns internal representation of original text
     * \return text after BiDi transformations without reordering
     */
    const WideString& GetPreparedText() const;

    /**
     * \brief Returns visual representation of original text
     * \param[in] trimEnd true for trims whitespace characters on line end
     * \return text after BiDi transformations with reordering and removing non-printable characters
     */
    const WideString GetVisualText(const bool trimEnd) const;

    /**
     * \brief Returns internal representation of last split line
     * \return last split line after BiDi transformations without reordering
     */
    const WideString& GetPreparedLine() const;

    /**
     * \brief Returns visual representation of last split line
     * \param[in] trimEnd true for trims whitespace characters on line end
     * \return last after BiDi transformations with reordering and removing non-printable characters
     */
    const WideString GetVisualLine(const bool trimEnd) const;

    /**
     * \brief Fill vector of split lines with specified width
     * \param[out] outputList vector of slit lines
     * \param[in] lineWidth maximum line width in pixels
     * \param[in] splitBySymbols true for split only by symbols
     * \param[in] trimEnd true for trims whitespace characters on each line end
     */
    void FillList(Vector<WideString>& outputList, float32 lineWidth, bool splitBySymbols, bool trimEnd);

private:
    /**
     * \brief Returns string after BiDi reordering and removing non-printable characters
     * \param[in] input string for processing
     * \param[in] trimEnd true for trims whitespace characters on line end
     * \return string after BiDi transformations with reordering and removing non-printable characters
     */
    const WideString BuildVisualString(const WideString& input, const bool trimEnd) const;

    /**
     * \brief Correct sizes for special BiDi characters
     */
    void PrepareCharSizes();

    WideString inputText;
    WideString preparedText;
    WideString preparedLine;
    
    bool useBiDi;
    bool isRtl;
    Vector<float32> characterSizes;
    Vector<uint8> breaks;
    BiDiHelper bidiHelper;
    size_t fromPos;
};

inline const WideString& TextLayout::GetPreparedLine() const
{
    return preparedLine;
}

inline const WideString& TextLayout::GetOriginalText() const
{
    return inputText;
}

inline const WideString& TextLayout::GetPreparedText() const
{
    return preparedText;
}

inline const bool TextLayout::IsRtlText() const
{
    return useBiDi && isRtl;
}

inline const uint32 TextLayout::Tell() const
{
    return static_cast<uint32>(fromPos);
}
}

#endif // __DAVAENGINE_TEXT_LAYOT_H__
