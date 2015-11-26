import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import WGControls 1.0

Item
{
    id : root
    property var node
    property var nodeContextMenu
    property real margin : 4.0

    x : node.nodePosX
    y : node.nodePosY
    width : contentRect.width
    height : contentRect.height

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

            MouseArea
            {
                id: dragHandle
                anchors.fill : parent
                acceptedButtons: Qt.LeftButton
                preventStealing: true

                property var mouseDragStart

                onPositionChanged:
                {
                    if(mouseDragStart && (mouse.buttons & Qt.LeftButton))
                    {
                        var pos = mapToItem(graphCanvasObject, mouse.x, mouse.y)
                        var delta = Qt.point(pos.x - mouseDragStart.x, pos.y - mouseDragStart.y)
                        node.shiftNode(delta.x, delta.y)
                        mouseDragStart = pos
                    }
                }

                onPressed : 
                {
                    mouseDragStart = mapToItem(graphCanvasObject, mouse.x, mouse.y)
                }

                onReleased : 
                {
                    mouseDragStart = null
                }
            }

            ContextMenu
            {
                id : contextMenu
                z : root.z
                menuModel : nodeContextMenu
                contextObjectUid : node.uid
            }
        
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

                        WGListModel
                        {
                            id : inputSlotsModel
                            source : node.inputSlots
                            ValueExtension {}
                        }

                        Repeater
                        {
                            id : inputSlotsRepeater
                            model : inputSlotsModel
                            delegate : InputSlot
                            {
                                z : root.z + 10
                                nodeObject : root
                                slotObject : Value

                            }
                        }
                    }

                    ColumnLayout
                    {
                        Item
                        {
                            width : 30
                        }
                    }

                    ColumnLayout
                    {
                        Layout.alignment : Qt.AlignTop | Qt.AlignRight
                        WGListModel
                        {
                            id : outputSlotsModel
                            source : node.outputSlots
                            ValueExtension {}
                        }

                        Repeater
                        {
                            id : outputSlotsRepeater
                            model : outputSlotsModel
                            delegate : OutputSlot
                            {
                                z : root.z + 10
                                nodeObject : root
                                slotObject : Value
                            }
                        }
                    }
                }
            }
        }
    }
}