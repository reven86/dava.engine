import QtQuick 2.1
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.2

Rectangle
{
    anchors.fill: parent.fill
    color: palette.MainWindowColor

    property color majorLineColor: palette.MidLightColor
    property color minorLineColor: palette.MidDarkColor
    property color backgroundColor: "grey"

    property var title: "Graph Editor"
    property var layoutHints: { 'left': 0.5 }

    ColumnLayout
    {
        id: content
        spacing: 2
        anchors.fill: parent

        Canvas
        {
            id: graphCanvas;
            contextType: "2d";
            Layout.fillHeight: true
            Layout.fillWidth: true
            focus: true

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

                // -- Text
                ctx.font = '12px Courier New';
                ctx.strokeStyle = majorLineColor;
                for (var i=startY;i<endY;i+=lineHeight)
                {
                    if(isMajor(i, lineHeight))
                    {
                        var y = (viewTransform.transformY(i) - 1);
                        ctx.strokeText((i * yScale).toPrecision(3), 20, y);
                    }
                }
            }
        }

        Rectangle
        {
                id: mouseLine;
                height: parent.height
                width: 1
                color: majorLineColor
        }

        MouseArea
        {
            anchors.fill: parent;
            acceptedButtons: Qt.AllButtons

            property var mouseDragStart;

            onWheel:
            {
                var delta = 1 + wheel.angleDelta.y/120.0 * .1;
                // Zoom into the current mouse location
                var screenPos = Qt.point(wheel.x, wheel.y)
                var oldPos = graphCanvas.viewTransform.inverseTransform(screenPos);
                graphCanvas.viewTransform.xScale *= delta;
                graphCanvas.viewTransform.yScale *= delta;
                var newScreenPos = graphCanvas.viewTransform.transform(Qt.point(oldPos.x, oldPos.y));
                var shift = Qt.point(screenPos.x - newScreenPos.x, screenPos.y - newScreenPos.y)
                graphCanvas.viewTransform.shift(shift);
                graphCanvas.requestPaint()
            }

            hoverEnabled: true
            onPositionChanged:
            {
                mouseLine.x = mouse.x
                if(mouseDragStart && (mouse.buttons & Qt.MiddleButton))
                {
                    var pos = Qt.point(mouse.x, mouse.y)
                    var delta = Qt.point(pos.x - mouseDragStart.x, pos.y - mouseDragStart.y)
                    graphCanvas.viewTransform.origin.x += delta.x
                    graphCanvas.viewTransform.origin.y += delta.y
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
    }
}