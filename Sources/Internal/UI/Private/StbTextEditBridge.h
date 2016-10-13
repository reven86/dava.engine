#ifndef __DAVA_UITEXTFIELDSTBBRIDGE_H__
#define __DAVA_UITEXTFIELDSTBBRIDGE_H__

#include "Base/BaseTypes.h"
#include "Math/Vector.h"
#include "Input/KeyboardDevice.h"

namespace DAVA
{
class TextBox;
class UIEvent;
struct StbState;

/**
 * \brief Class that implements bridge for stb_textedit
 */
class StbTextEditBridge
{
public:
    /**
     * \brief Delegate to implement stb_textedit callbacks
     */
    class StbTextDelegate
    {
    public:
        /** 
         * \brief Default destructor
         */
        virtual ~StbTextDelegate() = default;

        /**
        * \brief Service function for insert text in data structure
        * \param[in] position position of inserting
        * \param[in] str string to inserting
        * \param[in] length string length
        * \return count of inserted characters
        */
        virtual uint32 InsertText(uint32 position, const WideString::value_type* str, uint32 length) = 0;

        /**
        * \brief Service function for delete text from data structure
        * \param[in] position position of deleting
        * \param[in] length deleting substring length
        * \return count of deleted characters
        */
        virtual uint32 DeleteText(uint32 position, uint32 length) = 0;

        /**
        * \brief Service function for getting instance of TextBox from field
        * \return pointer to TextBox
        */
        virtual const TextBox* GetTextBox() const = 0;

        /**
        * \brief Service function for getting text length
        * \return text length
        */
        virtual uint32 GetTextLength() const = 0;

        /**
        * \brief Service function for getting character from text
        * \param[in] i character index
        * \return character
        */
        virtual WideString::value_type GetCharAt(uint32 i) const = 0;

        virtual WideString GetText() const = 0;

        virtual bool IsCharAvaliable(WideString::value_type ch) const = 0;
    };

    /**
     * \brief Default constructor
     */
    StbTextEditBridge(StbTextDelegate* delegate);

    /**
     * \brief Copy constructor
     * \param[in] c original object
     */
    StbTextEditBridge(const StbTextEditBridge& c);

    /**
     * \brief Destructor
     */
    virtual ~StbTextEditBridge();

    /**
     * \brief Copy class data to correct instance
     * \param[in] c object to copy data
     */
    virtual void CopyStbStateFrom(const StbTextEditBridge& c);

    /**
    
    */
    virtual bool SendKey(Key key, uint32 modifiers);

    /**
     * \brief Send key to STB text edit
     * \param[in] codePoint key code
     */
    virtual bool SendKeyChar(uint32 keyChar, uint32 modifiers);

    virtual bool SendRaw(uint32 codePoint);

    /**
    * \brief Send mouse click to STB text edit
    * \param[in] point mouse point (x,y) in control's local coordinates
    */
    virtual void Click(const Vector2& point);

    /**
    * \brief Send mouse drag event to STB text edit
    * \param[in] point mouse point (x,y) in control's local coordinates
    */
    virtual void Drag(const Vector2& point);

    /**
     * \brief Cut (delete) selected text
     */
    virtual bool Cut();

    /**
     * \brief Insert (replace selected) new text in field
     * \param[in] str string to pasting
     */
    virtual bool Paste(const WideString& str);

    /**
     * \brief Clear STB text edit undo stack
     */
    virtual void ClearUndoStack();

    /**
     * \brief Returns character index of selection start
     * \return character index
     */
    uint32 GetSelectionStart() const;

    /**
     * \brief Move start selection to position
     *        Cursor equal 0 - cursor before first symbol,
     *        cursor equal text length - cursor after last symbol
     * \param[in] position new start selection position
     */
    void SetSelectionStart(uint32 position) const;

    /**
     * \brief Returns character index of selection end
     * \return character index
     */
    uint32 GetSelectionEnd() const;

    /**
     * \brief Move end selection to position
     *        Cursor equal 0 - cursor before first symbol,
     *        cursor equal text length - cursor after last symbol
     * \param[in] position new end selection position
     */
    void SetSelectionEnd(uint32 position) const;

    /**
     * \brief Returns character index of cursor position. 
     *        Cursor equal 0 - cursor before first symbol, 
     *        cursor equal text length - cursor after last symbol
     * \return character index
     */
    uint32 GetCursorPosition() const;

    /**
    * \brief Move cursor to position.
    *        Cursor equal 0 - cursor before first symbol,
    *        cursor equal text length - cursor after last symbol
    * \param[in] position new cursor position
    */
    void SetCursorPosition(uint32 position) const;

    /**
    * \brief Enable single line mode
    * \param[in] signleLine flag of single line mode enabling
    */
    void SetSingleLineMode(bool signleLine);

    /**
    * \brief Return single line mode flag
    * \return if True that single line mode is enabled
    */
    bool IsSingleLineMode() const;

    /**
     * \brief Return inserting mode flag
     * \return if True that inserting mode is enabled
     */
    bool IsInsertMode() const;

    /**
    
    */
    void SelectWord();

    /**

    */
    void SelectAll();

    bool CutToClipboard();
    bool CopyToClipboard();
    bool PasteFromClipboard();

    /**
     * \brief Return delegate
     * \return delegate
     */
    StbTextDelegate* GetDelegate() const;

private:
    StbState* stb_state = nullptr; //!< Inner STB state structure ptr
    StbTextDelegate* delegate = nullptr; //!< Inner delegate to implement stb callbacks
};

inline StbTextEditBridge::StbTextDelegate* StbTextEditBridge::GetDelegate() const
{
    return delegate;
}
}

#endif //__DAVA_UITEXTFIELDSTBBRIDGE_H__