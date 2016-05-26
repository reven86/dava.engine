#ifndef __DAVA_UITEXTFIELDSTBBRIDGE_H__
#define __DAVA_UITEXTFIELDSTBBRIDGE_H__

#include "Base/BaseTypes.h"
#include "Math/Vector.h"

namespace DAVA
{
class TextBox;
struct StbState;

/**
 * \brief Class that implements bridge for stb_textedit
 */
class StbTextEditBridge
{
public:
    /**
     * \brief Control keys in stb_textedit
     */
    enum CotrolKeys : uint32
    {
        KEY_SHIFT_MASK = 0x00020000,
        KEY_LEFT = 0x00010000,
        KEY_RIGHT = 0x00010001,
        KEY_UP = 0x00010002,
        KEY_DOWN = 0x00010003,
        KEY_LINESTART = 0x00010004,
        KEY_LINEEND = 0x00010005,
        KEY_TEXTSTART = 0x00010006,
        KEY_TEXTEND = 0x00010007,
        KEY_DELETE = 0x00010008,
        KEY_BACKSPACE = 8,
        KEY_UNDO = 0x00010009,
        KEY_REDO = 0x00010010,
        KEY_INSERT = 0x00010011,
        KEY_WORDLEFT = 0x00010012,
        KEY_WORDRIGHT = 0x00010013,
    };

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
        virtual const TextBox* GetTextBox() = 0;

        /**
        * \brief Service function for getting text length
        * \return text length
        */
        virtual uint32 GetTextLength() = 0;

        /**
        * \brief Service function for getting character from text
        * \param[in] i character index
        * \return character
        */
        virtual WideString::value_type GetCharAt(uint32 i) = 0;
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
     * \brief Send key to STB text edit
     * \param[in] codePoint key code
     */
    virtual void SendKey(uint32 codePoint);

    /**
     * \brief Cut (delete) selected text
     */
    virtual void Cut();

    /**
     * \brief Insert (replace selected) new text in field
     * \param[in] str string to pasting
     */
    virtual void Paste(const WideString& str);

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