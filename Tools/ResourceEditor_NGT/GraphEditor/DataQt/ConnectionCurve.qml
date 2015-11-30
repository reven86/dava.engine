import QtQuick 2.0

Canvas
{
    anchors.fill: parent
    antialiasing : true

    property real _xScale
    property real _yScale
    property var _origin

    property var startPoint
    property var endPoint
    property color curveColor

    onPaint:
    {
        var ctx = getContext('2d');
        ctx.clearRect(0, 0, width, height);

        if (startPoint == null || endPoint == null)
            return;

        ctx.lineWidth = 6.0
        ctx.lineCap = "round"
        ctx.strokeStyle = Qt.rgba(curveColor.r, curveColor.g, curveColor.b, 0.3)

        ctx.beginPath()
        ctx.moveTo(startPoint.x, startPoint.y)
        ctx.lineTo(endPoint.x, endPoint.y)
        ctx.stroke();

        // -- Solid line
        ctx.lineWidth = 2.0
        ctx.lineCap = "round"
        ctx.strokeStyle = curveColor

        ctx.beginPath()
        ctx.moveTo(startPoint.x, startPoint.y)
        ctx.lineTo(endPoint.x, endPoint.y)
        ctx.stroke();
    }
}
