import QtQuick 2.4
import TArcControls 1.0

NumberBox
{
    id: numberBox
    property var dataContext

    anchors.fill: parent
    number: dataContext != null ? dataContext.value : 0
    minimumValue: dataContext != null ? dataContext.minValue : 0
    maximumValue: dataContext != null ? dataContext.maxValue : 0
    stepSize: 1
    decimals: 0
    readOnly: dataContext != null ? dataContext.readOnly : true

    onNumberChanged:
    {
        ValueSetHelper.SetValue(dataContext, "value", numberBox.number);
    }
}

