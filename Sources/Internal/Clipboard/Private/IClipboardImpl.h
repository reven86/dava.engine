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

#ifndef __DAVAENGINE_ICLIPBOARDIMPL_H__
#define __DAVAENGINE_ICLIPBOARDIMPL_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
/**
 * \brief Interface to implement platform clipboard helper
 */
class IClipboardImpl
{
public:
    /**
     * \brief Destructor
     */
    virtual ~IClipboardImpl() = default;

    /**
     * \brief Return status of clipboard helper
     * \return true if helper ready to work with clipboard
     */
    virtual bool IsReadyToUse() const = 0;

    /**
     * \brief Clear system clipboard
     */
    virtual void ClearClipboard() const = 0;

    /**
     * \brief Copy to system clipboard WideString as Unicode string
     * \param[in] str input string
     */
    virtual void SetWideString(const WideString& str) const = 0;

    /**
     * \brief Get from system clipboard Unicode text data as WideString
     * \return WideString with clipboard content
     */
    virtual WideString GetWideString() const = 0;

    /**
     * \brief Copy to system clipboard String as ASCII/UTF-8 string
     * \param[in] str input string
     */
    virtual void SetString(const String& str) const = 0;

    /**
     * \brief Get from system clipboard ASCII/UTF-8 text data as String
     * \return String with clipboard contetn
     */
    virtual String GetString() const = 0;
};
}

#endif //__DAVAENGINE_ICLIPBOARDIMPL_H__
