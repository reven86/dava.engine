import QtQuick 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4

Rectangle
{
    ColumnLayout
    {
        anchors.fill:parent
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
            model: context.fileSystemModel
            onDoubleClicked:
            {
                context.openScene(index);
            }
        }
    }
}
