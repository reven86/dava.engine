import QtQuick 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4

Rectangle
{
    ColumnLayout
    {
        anchors.fill:parent
        RowLayout
        {
            TextEdit
            {
                anchors.left: parent.left
                anchors.right: pushButton.left
                id:textEdit
                text: context.sampleText
                font.family: "Helvetica"
                color: "black"
                focus: true

                onTextChanged:
                {
                    context.sampleText = text
                }
            }

            Button
            {
                anchors.right: parent.right
                id:pushButton
                text:"Button"
            }
        }

        TreeView
        {
            Layout.fillWidth:true
            Layout.fillHeight:true
            TableViewColumn
            {
                title: "Name"
                role: "fileName"
                width: 300
            }
            TableViewColumn
            {
                title: "Permissions"
                role: "filePermissions"
                width: 100
            }
            model: context != null ? context.fileSystemModel : null
        }
    }
}
