import QtQuick 2.4
import QtQuick.Controls 1.4 as QtControls

QtControls.SpinBox
{
    id: spinBox
    objectName: "NumberBox"

    /*!
        This property contains the value represented by the control
         The default value is \c 0
    */
    property real number: 0
    property bool multipleValues: false
    property bool readOnly: false

    value: number
    activeFocusOnPress: readOnly

    onEditingFinished:
    {
        ValueSetHelper.SetValue( spinBox, "number", value );
    }

    onNumberChanged:
    {
        value = number;
    }
}
