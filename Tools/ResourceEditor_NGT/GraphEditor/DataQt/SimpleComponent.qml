import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import WGControls 1.0

Item
{
    id : root
    property var node
    property real margin : 4.0

    x : node.nodePosX + width / 2.0
    y : node.nodePosY + height / 2.0
    transform : Scale
    {
        origin.x : 0
        origin.y : 0
        xScale : node.nodeScale
        yScale : node.nodeScale
    }

    Rectangle
    {
        id : border
        width : contentRect.width + margin
        height : contentRect.height + margin
        color : "black"

        Rectangle
        {
            anchors.centerIn : parent
            id : contentRect
            width : mainLayout.width
            height : mainLayout.height
        
            LinearGradient 
            {
                anchors.fill: parent
                start: Qt.point(0, parent.height)
                end: Qt.point(parent.width, 0)
                gradient: Gradient
                {
                    GradientStop { position: 0.0; color: "white" }
                    GradientStop { position: 1.0; color: "black" }
                }
            }

            WGColumnLayout
            {
                id : mainLayout
                Text
                {
                    id : header
                    Layout.fillHeight : true
                    Layout.alignment : Qt.AlignHCenter | Qt.AlignVCenter
                    text : node.title
                }

                Rectangle
                {
                    Layout.fillWidth : true
                    Layout.maximumHeight : margin / 2.0
                    Layout.minimumHeight : margin / 2.0
                    Layout.preferredHeight : margin / 2.0
                    color : "black"
                }

                WGExpandingRowLayout
                {
                    ColumnLayout
                    {
                        Layout.alignment : Qt.AlignTop | Qt.AlignLeft
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

                            WGColorButton {}
                        }
                        //ConnectonSlot {}
                    }

                    ColumnLayout
                    {
                        Layout.alignment : Qt.AlignTop | Qt.AlignRight
                        //ConnectonSlot {}
                        //ConnectonSlot {}
                    }
                }
            }
        }
    }
}