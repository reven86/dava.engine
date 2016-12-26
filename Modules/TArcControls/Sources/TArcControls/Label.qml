import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

Text
{
    id: labelText
    objectName: "Label"

    /*! This property right aligns the label and sets width to the largest label in the panel.
        The default value is false
    */
    property bool formLabel: false

    /*! This property ignores the panel wide label column width
        The default is false
    */
    property bool localForm: false

    property QtObject linkedFormObject: Item{}

    enabled: linkedFormObject.enabled


    /*
        Links the label to it's control object and then finds the copyable inside it.
        Only works with form labels.
        @param type:object parentObject The parent control object
    */
    /*! \internal */
    function selectLabelControl(parentObject)
    {
        for (var i=0; i<parentObject.children.length; i++)
        {
            if (parentObject.children[i].label_ == labelText.text)
            {
                selectControlCopyable(parentObject.children[i])
                break;
            }
            else
            {
                selectLabelControl(parentObject.children[i])
            }
        }
    }

    function selectControlCopyable(parentObject){
        for (var i=0; i<parentObject.children.length; i++)
        {
            if (typeof parentObject.children[i].rootCopyable != "undefined")
            {
                formControlCopyable_ = parentObject.children[i]
            }
        }
    }

    Component.onCompleted:
    {
        if (formLabel && paintedWidth > defaultSpacing.labelColumnWidth && !localForm)
        {
            defaultSpacing.labelColumnWidth = paintedWidth;
        }

        if (formLabel)
        {
            selectLabelControl(labelText.parent)
        }
    }

    width: formLabel && !localForm ? defaultSpacing.labelColumnWidth: implicitWidth

    Layout.preferredWidth: formLabel && !localForm ? defaultSpacing.labelColumnWidth : implicitWidth

    color: enabled ? palette.textColor : palette.disabledTextColor

    renderType: Text.NativeRendering

    smooth: true

    horizontalAlignment: formLabel ? Text.AlignRight : Text.AlignLeft

    MouseArea
    {
        anchors.fill: parent
        enabled: labelText.formLabel
        hoverEnabled: labelText.formLabel
        cursorShape: labelText.formLabel ? Qt.PointingHandCursor : Qt.ArrowCursor

        onClicked:
        {
            if ((formControlCopyable_ === null) || (!formControlCopyable_.enabled))
            {
                return;
            }
            if (!globalSettings.wgCopyableEnabled)
            {
                return;
            }

            if ((mouse.button == Qt.LeftButton) && (mouse.modifiers & Qt.ControlModifier))
            {
                if (formControlCopyable_.selected)
                {
                    formControlCopyable_.deSelect()
                }
                else
                {
                    formControlCopyable_.select()
                }
            }
            else if (mouse.button == Qt.LeftButton)
            {
                formControlCopyable_.rootCopyable.deSelect();
                formControlCopyable_.select()
            }
        }
    }
}
