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
     * \return true if successful 
     */
    virtual bool ClearClipboard() const = 0;

    /**
     * \brief Check that system clipboard contains Unicode text
     * \return true if system clipboard contains Unicode text
     */
    virtual bool HasText() const = 0;

    /**
     * \brief Copy to system clipboard WideString as Unicode string
     * \param[in] str input string
     * \return true if successful
     */
    virtual bool SetText(const WideString& str) = 0;

    /**
     * \brief Get from system clipboard Unicode text data as WideString
     * \return WideString with clipboard content
     */
    virtual WideString GetText() const = 0;
};
}

#endif //__DAVAENGINE_ICLIPBOARDIMPL_H__
