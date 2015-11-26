import QtQuick 2.1
import QtQuick.Layouts 1.1
import WGControls 1.0

Slot
{
    id : inputSlot
    isInput : true

    width : slotLayout.width
    height : slotLayout.height

    WGExpandingRowLayout
    {
        id : slotLayout
        SlotImage
        {
            z : inputSlot.z
            Layout.alignment : Qt.AlignTop | Qt.AlignLeft
            slot : inputSlot
        }

        Text
        {
            Layout.alignment : Qt.AlignTop | Qt.AlignLeft
            text: slotObject.title
        }
    }
}