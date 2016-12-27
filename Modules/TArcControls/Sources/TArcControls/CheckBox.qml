import QtQuick 2.4
import QtQuick.Controls 1.4 as QtControls

import TArcControls.Styles 1.0

QtControls.CheckBox
{
    id: checkBox
    objectName: "CheckBox"

    property bool checked: false
    property bool multipleValues : false

    partiallyCheckedEnabled : multipleValues
    checkedState: multipleValues ? Qt.PartiallyChecked : checked ? Qt.Checked : Qt.Unchecked

    activeFocusOnTab: enabled
    implicitHeight: defaultSpacing.minimumRowHeight

    onCheckedStateChanged:
    {
        if (checkedState === Qt.PartiallyChecked)
        {
            ValueSetHelper.SetValue(checkBox, "checked", false);
            ValueSetHelper.SetValue(checkBox, "multipleValues", false);
        }
        else
        {
            ValueSetHelper.SetValue(checkBox, "checked", checkedState === Qt.Checked);
        }
    }

    Component.onCompleted:
    {
        implicitWidth = Math.round(implicitWidth)
    }

    style: CheckStyle
    {
    }
}
