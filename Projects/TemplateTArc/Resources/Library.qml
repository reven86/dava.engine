import QtQuick 2.3
import QtQuick.Controls 1.4

Rectangle
{
    color:"red"
    TreeView
    {
        anchors.fill : parent
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
