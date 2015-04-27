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
     * Wrap modes for text splitting
     */
    enum WrapMode {
        WRAP_BY_WORDS = 0, //!< Wrap by words
        WRAP_BY_SYMBOLS //!<  Wrap by symbols
    };

    /**
     * \brief Create TextLayout with word wrap and disabled bidi transformations
     */
    TextLayout();

    /**
     * \brief Create TextLayout with specified wrap mode and bidi transformations
     * \param[in] wrapMode selected wrap mode (WRAP_BY_WORD or WRAP_BY_SYMBOLS)
     * \param[in] useBiDi true for enabling bidi transformations
     */
    TextLayout(const WrapMode wrapMode, const bool useBiDi);

    /**
     * \brief Destructor
     */
    virtual ~TextLayout();

    /**
     * \brief Set current wrap mode
     * \param[in] mode selected wrap mode, \see WrapMode
     */
    void SetWrapMode(const WrapMode mode);

    /**
     * \brief Return current wrap mode
     * \return current wrap mode
     */
    const WrapMode GetWrapMode() const;

    /**
     * \brief Set process text and font for text splitting
     * \param[in] input text to process
     * \param[in] font font for detecting characters sizes
     */
    void Reset(const WideString& input, const Font& font);

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
     * \return true if not all text processed
     */
    bool HasNext();

    /**
     * \brief Process next text block from current cursor to end and get possible line with specified width or less
     * \param[in] lineWidth maximum of line width in pixels
     */
    void Next(const float32 lineWidth);

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
     * \return text after bidi transformations without reordering
     */
    const WideString& GetPreparedText() const;

    /**
     * \brief Returns visual representation of original text
     * \param[in] trimEnd true for trims whitespace characters on line end
     * \return text after bidi transformations with reordering and removing non-printable characters
     */    
    const WideString GetVisualText(const bool trimEnd) const;

    /**
     * \brief Returns internal representation of last split line
     * \return last split line after bidi transformations without reordering
     */
    const WideString& GetPreparedLine() const;

    /**
     * \brief Returns visual representation of last split line
     * \param[in] trimEnd true for trims whitespace characters on line end
     * \return last after bidi transformations with reordering and removing non-printable characters
     */
    const WideString GetVisualLine(const bool trimEnd) const;

private:
    /**
     * \brief Returns string after bidi reordering and removing non-printable characters
     * \param[in] input string for processing
     * \param[in] trimEnd true for trims whitespace characters on line end
     * \return string after bidi transformations with reordering and removing non-printable characters
     */
    const WideString BuildVisualString(const WideString& input, const bool trimEnd) const;

    /**
     * \brief Split text by words from current cursor position with specified width
     * \param[in] lineWidth maximum line width in pixels
     */
    void NextByWords(const float32 lineWidth);

    /**
     * \brief Split text by symbols from current cursor position with specified width
     * \param[in] lineWidth maximum line width in pixels
     */
    void NextBySymbols(const float32 lineWidth);

    WideString inputText;
    WideString preparedText;
    WideString preparedLine;
    
    WrapMode wrapMode;
    bool useBiDi;
    bool isRtl;
    Vector<float32> characterSizes;
    Vector<uint8> breaks;
    BiDiHelper bidiHelper;
    uint32 fromPos;
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

inline const TextLayout::WrapMode TextLayout::GetWrapMode() const
{
    return wrapMode;
}

inline const uint32 TextLayout::Tell() const
{
    return fromPos;
}


}