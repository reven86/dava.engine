import QtQuick 2.3
import QtQuick.Layouts 1.1
import WGControls 1.0

WGExpandingRowLayout
{
    id: root
    WGListModel
    {
        id: buttonsModel
        source: itemData != null ? itemData.buttonsDefinition : null
        ButtonsModelExtension{}
    }

    Repeater
    {
        model: buttonsModel
        delegate: WGPushButton
        {
            id: button
            Layout.preferredHeight: defaultSpacing.minimumRowHeight
            Layout.preferredWidth: defaultSpacing.minimumRowHeight
            iconSource: buttonIcon
            enabled: buttonEnabled

            onClicked:
            {
                buttonClicked = true;
            }
        }
    }

    Loader
    {
        Layout.fillWidth: true
        Layout.preferredHeight: defaultSpacing.minimumRowHeight
        sourceComponent: itemData != null ? itemData.Component : null
    }
}
