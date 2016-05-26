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
