import QtQuick 2.1
import QtQuick.Layouts 1.1
import WGControls 1.0

Slot
{
    id : outputSlot
    isInput : false

    width : slotLayout.width
    height : slotLayout.height

    WGExpandingRowLayout
    {
        id : slotLayout
        Text
        {
            Layout.alignment : Qt.AlignTop | Qt.AlignRight
            text: slotObject.title
        }

        WGTextBox
        {
            placeholderText: "empty text"
            width : 30
            assetBrowserContextMenu: false
        }

        WGSpinBox
        {
            width: 120
            value: 25
            minimumValue: 0
            maximumValue: 100
        }

        SlotImage
        {
            z : outputSlot.z
            Layout.alignment : Qt.AlignTop | Qt.AlignRight
            slot : outputSlot
        }
    }
}