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

        SlotImage
        {
            z : outputSlot.z
            Layout.alignment : Qt.AlignTop | Qt.AlignRight
            slot : outputSlot
        }
    }
}