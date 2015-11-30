import QtQuick 2.1
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.2
import WGControls 1.0

Rectangle
{
    id: graphEditorComponent
    objectName: "GraphEditorView"
    color: palette.MainWindowColor

    property color majorLineColor: palette.MidLightColor
    property color minorLineColor: palette.MidDarkColor
    property color backgroundColor: "grey"

    property var title: "Graph Editor"
    property var layoutHints: { 'left': 0.5 }

    property var slotsIndex : []
    property var graphCanvasObject : graphCanvas
    property var connectionStartSlot
    property var interactiveConnectionCurve : connectionCurve

    onWidthChanged : { sizeChanged(width, height) }
    onHeightChanged : { sizeChanged(width, height) }

    function connect(output, input)
    {
        createConnection(output, input)
    }

    function getNodeContextMenu()
    {
        return nodeMenuModel
    }

    function getSlotContextMenu()
    {
        return slotMenuModel
    }

    ColumnLayout
    {
        id: content
        spacing: 2
        anchors.fill: parent

        Canvas
        {
            id: graphCanvas;
            contextType: "2d";
            z : 0
            Layout.fillHeight: true
            Layout.fillWidth: true

            property var viewTransform: ViewTransform
            {
                container: graphCanvas
            }

            function nearestQuarter(val)
            {
                if( val < 1 )
                {
                    // Find nearest multiple of 1/4
                    var multiplier = 4;
                    while(val * multiplier < 1)
                    {
                        multiplier *= 4;
                    }
                    val = 1/multiplier;
                }
                else if(val < 4)
                    val = 1;
                else
                {
                    // Find nearest multiple of 4
                    val = Math.floor(val) - (Math.floor(val) % 4);
                }
                return val
            }

            function isMajor(val, lineHeight)
            {
                var mod4 = (Math.abs(val / lineHeight) % 4)
                return mod4 < 0.000001;
            }

            onPaint:
            {
                var height = graphCanvas.height;
                var ctx = graphCanvas.getContext('2d');
                ctx.fillStyle = backgroundColor
                ctx.fillRect(0, 0, graphCanvas.width, graphCanvas.height);

                var startY = graphCanvas.viewTransform.inverseTransform(Qt.point(0,height)).y
                var endY = graphCanvas.viewTransform.inverseTransform(Qt.point(0,0)).y
                // The maximum number of pixels between lines
                var pixelGap = 20
                var numlines = (height / pixelGap)
                var lineHeight = nearestQuarter((endY - startY) / numlines);
                var nearStartWhole = Math.floor(startY) - Math.floor(startY) % lineHeight
                var countFromWhole = Math.floor((startY - nearStartWhole) / lineHeight)
                startY = nearStartWhole + countFromWhole * lineHeight;

                // -- Dark lines
                ctx.beginPath();
                ctx.strokeStyle = minorLineColor;
                for (var i = startY; i < endY; i += lineHeight)
                {
                    if(!isMajor(i, lineHeight))
                    {
                        var y = viewTransform.transformY(i);
                        ctx.moveTo(0, Math.floor(y) + 0.5);
                        ctx.lineTo(graphCanvas.width, Math.floor(y) + 0.5);
                    }
                }
                ctx.stroke();

                // -- Darker lines
                ctx.beginPath();
                ctx.strokeStyle = majorLineColor;
                for (var i = startY; i < endY; i += lineHeight)
                {
                    if(isMajor(i, lineHeight))
                    {
                        var y = viewTransform.transformY(i);
                        ctx.moveTo(0, Math.floor(y) + 0.5);
                        ctx.lineTo(graphCanvas.width, Math.floor(y) + 0.5);
                    }
                }
                ctx.stroke();
            }
            
            MouseArea
            {
                anchors.fill: parent;
                acceptedButtons: Qt.AllButtons
                z : graphCanvas.z
                
                property var mouseDragStart;
                
                onWheel:
                {
                    var delta = 1 + wheel.angleDelta.y/120.0 * .1;
                    // Zoom into the current mouse location
                    var screenPos = Qt.point(wheel.x, wheel.y)
                    var oldPos = graphCanvas.viewTransform.inverseTransform(screenPos);
                    graphCanvas.viewTransform.scale(delta)
                    var newScreenPos = graphCanvas.viewTransform.transform(Qt.point(oldPos.x, oldPos.y));
                    var shift = Qt.point(screenPos.x - newScreenPos.x, screenPos.y - newScreenPos.y)
                    graphCanvas.viewTransform.shift(shift);
                    scaleCanvas(delta, wheel.x, wheel.y);
                    graphCanvas.requestPaint()
                }
                
                onPositionChanged:
                {
                    if(mouseDragStart && (mouse.buttons & Qt.MiddleButton))
                    {
                        var pos = Qt.point(mouse.x, mouse.y)
                        var delta = Qt.point(pos.x - mouseDragStart.x, pos.y - mouseDragStart.y)
                        shiftCanvas(delta.x, delta.y)
                        mouseDragStart = pos
                        graphCanvas.requestPaint()
                    }
                }
                
                onPressed:
                {
                    mouseDragStart = Qt.point(mouse.x, mouse.y)
                }
                
                onReleased:
                {
                    mouseDragStart = null;
                }
            }

            WGListModel
            {
                id : nodesModel
                source : nodes

                ValueExtension {}
            }
            
            Repeater
            {
                id: itemRepeater
                model: nodesModel
                delegate: SimpleComponent
                {
                    z : graphCanvas.z + 10
                    node : Value
                    nodeContextMenu : getNodeContextMenu()
                }
            }

            WGListModel
            {
                id: connectorsMode
                source: connectors

                ValueExtension {}
            }

            Repeater
            {
                id: connectorRepeater
                model: connectorsMode
                delegate: ConnectionCurve
                {
                    curveColor : "red"
                    z : graphCanvas.z + 100 // reserve some z range for nodes, slots and other stuff
                    isInteractive : false
                    outputSlot : slotsIndex[Value.outputSlot]
                    inputSlot : slotsIndex[Value.inputSlot]
                }
            }

            ConnectionCurve
            {
                id : connectionCurve
                curveColor : "red"
                z : graphCanvas.z + 110 // interactive curve always on top 
            }

            ContextMenu
            {
                z: graphCanvas.z
                menuModel : contextMenuModel
            }
        }
    }
}