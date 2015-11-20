import QtQuick 2.1
import QtQuick.Layouts 1.1
import WGControls 1.0

Item
{
    property var slotModel
    
    WGExpandingRowLayout
    {
        Text
        {
            text: "Output Slot1!"
        }
        
        Image
        {
            source: "qrc:/GE/greenSlot.png"
            sourceSize.width : 12
            sourceSize.height : 12
        }
    }
}