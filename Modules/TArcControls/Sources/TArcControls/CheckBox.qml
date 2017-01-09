import QtQuick 2.6
import QtQuick.Controls.Styles 1.4 as QtStyles
import QtQuick.Controls 1.4 as QtControls

import TArcControls 1.0

QtControls.CheckBox
{
    id: checkBox
    objectName: "CheckBox"

    activeFocusOnTab: enabled

    style: QtStyles.CheckBoxStyle
    {
    }
}
