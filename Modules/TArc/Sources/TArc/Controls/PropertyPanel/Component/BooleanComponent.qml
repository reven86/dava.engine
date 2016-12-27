import QtQuick 2.4
import TArcControls 1.0

CheckBox
{
    id: checkbox
    property var dataContext
    objectName: typeof dataContext.objectName == "undefined" ? "BooleanComponent" : dataContext.objectName
    anchors.left: parent.left
    checked: dataContext.value
    enabled: dataContext.enabled && !dataContext.readOnly
    multipleValues: dataContext.multipleValues

    onCheckedChanged:
    {
        dataContext.value = checked;
    }
}
