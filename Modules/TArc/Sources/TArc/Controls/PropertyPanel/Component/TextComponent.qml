import QtQuick 2.4
import TArcControls 1.0

TextBox
{
    id: textField
    property var dataContext
    objectName: typeof dataContext.objectName == "undefined" ? "TextComponent" : dataContext.objectName
    anchors.left: parent.left
    anchors.right: parent.right
    text: dataContext.text
    readOnly: dataContext.readOnly
    enabled: dataContext.enabled

    onEditAccepted:
    {
        dataContext.text = text;
    }
}

