import QtQuick 2.4
import TArcControls 1.0

TextBox
{
    id: textField
    property var dataContext
    anchors.fill: parent
    text: dataContext != null ? dataContext.text : ""
    readOnly: dataContext != null ? dataContext.readOnly : true
    enabled: dataContext != null ? dataContext.enabled : false
    selectTextOnAccepted: false

    onEditAccepted:
    {
        ValueSetHelper.SetValue(dataContext, "text", text);
    }
}

