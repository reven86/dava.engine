import QtQuick 2.4
import TArcControls 1.0

CheckBox
{
    id: checkbox
    property var dataContext
    anchors.fill: parent
    partiallyCheckedEnabled: dataContext != null ? dataContext.value == Qt.PartiallyChecked : false
    checkedState: dataContext != null ? dataContext.value : Qt.UnChecked
    enabled: dataContext!= null ? !dataContext.readOnly : false

    onCheckedStateChanged:
    {
        ValueSetHelper.SetValue(dataContext, "value", checkedState);
    }
}
