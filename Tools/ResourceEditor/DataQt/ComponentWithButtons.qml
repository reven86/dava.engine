import QtQuick 2.3
import QtQuick.Layouts 1.1
import WGControls 1.0

WGExpandingRowLayout
{
    id: root
    WGListModel
    {
        id: buttonsModel
        ButtonsModelExtension{}
    }

    Repeater
    {
        Component.onCompleted:
        {
            buttonsModel.source = itemData != null ? itemData.ButtonsDefinition : null
            model = buttonsModel;
        }

        delegate: WGPushButton
        {
            id: button
            Layout.preferredHeight: defaultSpacing.minimumRowHeight
            Layout.preferredWidth: defaultSpacing.minimumRowHeight
            iconSource: ButtonIcon
            enabled: ButtonEnabled

            onClicked:
            {
                itemData.ButtonClicked = true;
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
