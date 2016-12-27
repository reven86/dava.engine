import QtQuick 2.4
import QtQuick.Controls 1.4
import TArcControls.Styles 1.0

TextField
{
    id: textBox
    objectName: "TextBox"

    /*! This property determines if the textBox has a context menu
        The default value is \c true
    */
    property bool useContextMenu : true

    /*! This property determines if the text is selected when the user presses Enter or Return.
        The default value is \c true
    */
    property bool selectTextOnAccepted: true

    /*! This alias holds the width of the text entered into the textbox.
      */
    property alias contentWidth: contentLengthHelper.contentWidth

    property string prevText

    /*! This signal is emitted when text field loses focus and text changes is accepted */
    signal editAccepted();

    /*! This signal is emitted when text field editing is cancelled */
    signal editCancelled();

    /*! This property denotes if the control's text should be scaled appropriately as it is resized */
    smooth: true

    //Placeholder text in italics
    font.italic: text == "" ? true : false

    activeFocusOnTab: readOnly ? false : true
    verticalAlignment: TextInput.AlignVCenter

    // provide default heights
    implicitHeight: defaultSpacing.minimumRowHeight
    implicitWidth: contentLengthHelper.contentWidth + defaultSpacing.doubleMargin

    text: currentText

    /*! This function recalculates the width of the text box based on its contents */
    function recalculateWidth()
    {
        contentLengthHelper.text = textBox.text + "MM"
    }

    Keys.onPressed:
    {
        if (activeFocus)
        {
            if(event.key == Qt.Key_Enter || event.key == Qt.Key_Return)
            {
                textBox.focus = true;
                if (selectTextOnAccepted)
                {
                    selectAll();
                }
                editAccepted()
            }
            else if (event.key == Qt.Key_Escape)
            {
                ValueSetHelper.SetValue(textBox, "text", prevText);
                editCancelled();
                textBox.focus = true;
                recalculateWidth()
            }
        }
    }

    onActiveFocusChanged:
    {
        if (activeFocus)
        {
            selectAll()
            ValueSetHelper.SetValue(textBox, "prevText", text);
        }
        else
        {
            deselect()
            if (acceptableInput && (currentText !== prevText))
            {
                editAccepted();
            }
        }
    }

    onEditAccepted:
    {
        recalculateWidth()
    }

    onEditCancelled:
    {
        recalculateWidth()
    }

    Text
    {
        id: contentLengthHelper
        visible: false
        Component.onCompleted:
        {
            recalculateWidth()
        }
    }

    style: TextBoxStyle
    {
    }

    MouseArea
    {
        id: mouseAreaContextMenu
        acceptedButtons: Qt.RightButton

        propagateComposedEvents: true
        preventStealing: false
        anchors.fill: parent
        hoverEnabled: true

        cursorShape: Qt.IBeamCursor
        onClicked:
        {
            if (textBox.useContextMenu)
            {
                var highlightedText = selectedText
                contextMenu.popup()
            }
        }
    }

    Menu
    {
        id: contextMenu
        objectName: "Menu"
        title: "Edit"

        MenuItem
        {
            objectName: "Cut"
            text: "Cut"
            shortcut: "Ctrl+X"
            enabled: readOnly == true ? false : true
            onTriggered:
            {
                cut()
            }
        }

        MenuItem {
            objectName: "Copy"
            text: "Copy"
            shortcut: "Ctrl+C"
            onTriggered:
            {
                copy()
            }
        }

        MenuItem
        {
            objectName: "Paste"
            text: "Paste"
            shortcut: "Ctrl+V"
            enabled: canPaste == true ? true : false
            onTriggered:
            {
                paste()
            }
        }

        MenuSeparator { }

        MenuItem
        {
            objectName: "SelectAll"
            text: "Select All"
            shortcut: "Ctrl+A"
            onTriggered:
            {
                selectAll()
            }
        }
    }
}
