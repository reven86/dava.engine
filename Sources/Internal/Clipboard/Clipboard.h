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

#ifndef __DAVAENGINE_CLIPBOARD_H__
#define __DAVAENGINE_CLIPBOARD_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
class IClipboardImpl;

/**
 * \brief Helper to work with system clipboard
 */
class Clipboard
{
public:
    /**
     * \brief Constructor
     */
    Clipboard();

    /**
     * \brief Destructor
     */
    ~Clipboard();

    /**
     * \brief Return status of clipboard helper
     * \return true if helper ready to work with clipboard
     */
    bool IsReadyToUse() const;

    /**
     * \brief Clear system clipboard
     * \return true if successful
     */
    bool ClearClipboard() const;

    /**
     * \brief Check that system clipboard contains Unicode text
     * \return true if system clipboard contains Unicode text
     */
    bool HasText() const;

    /**
     * \brief Copy to system clipboard WideString as Unicode string
     * \param[in] str input string
     * \return true if successful
     */
    bool SetText(const WideString& str);

    /**
     * \brief Get from system clipboard Unicode text data as WideString
     * \return WideString with clipboard content
     */
    WideString GetText() const;

private:
    IClipboardImpl* pImpl;
};
}

#endif //__DAVAENGINE_CLIPBOARD_H__
