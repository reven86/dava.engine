import QtQuick 2.4
import QtQuick.Controls.Styles 1.4 as QtStyles
import QtQuick.Controls 1.4 as QtControls
import TArcControls 1.0

QtControls.SpinBox
{
    id: spinBox
    objectName: "NumberBox"

    /*!
        This property contains the value represented by the control
         The default value is \c 0
    */
    property real number: 0
    property bool readOnly: false

    value: number
    activeFocusOnPress: !readOnly

    onEditingFinished:
    {
        ValueSetHelper.SetValue( spinBox, "number", value );
    }

    style: QtStyles.SpinBoxStyle
    {
        horizontalAlignment: Qt.AlignLeft
        renderType: Text.NativeRendering
    }
}
