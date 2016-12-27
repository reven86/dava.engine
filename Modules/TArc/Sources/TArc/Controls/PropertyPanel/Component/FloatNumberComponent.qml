import QtQuick 2.4
import TArcControls 1.0

NumberBox
{
    id: numberBox
    property var dataContext
    objectName: typeof dataContext.objectName == "undefined" ? "FloatNumberComponent" : dataContext.objectName

    number: dataContext.value
    minimumValue: dataContext.minValue
    maximumValue: dataContext.maxValue
    stepSize: dataContext.stepSize
    decimals: dataContext.decimals
    readOnly: dataContext.readOnly
    enabled: dataContext.enabled
    multipleValues: dataContext.multipleValues

    onNumberChanged:
    {
        dataContext.value = numberBox.number;
    }
}

