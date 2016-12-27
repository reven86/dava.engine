import QtQuick 2.5
import QtQuick.Controls 1.4

import WGControls 2.0

WGExpandingComponent
{
    id: control
    objectName: typeof itemData.indexPath == "undefined" ? "angle_component" : itemData.indexPath
    enabled: itemData.enabled && !itemData.readOnly
    property var metaAngle: itemData.object.getMetaObject(itemData.name, "Angle");
    readonly property bool convertToRadians: (metaAngle && metaAngle.convertToRadians)

    replace: false

    function convertValue(value)
    {
        value = convertToRadians ? (value * 180 / Math.PI) : value
        // Don't allow comparing angles outside the valid range
        while(value < itemData.minValue)
            value += 360
        while(value > itemData.maxValue)
            value -= 360
        return value
    }

    property var angleValue: convertValue(itemData.value)

    onAngleValueChanged:
    {
        var currentValue = convertValue(itemData.value);
        if(currentValue != angleValue)
        {
            itemData.value = angleValue * (convertToRadians ? Math.PI / 180 : 1);;
        }
    }

    mainComponent: WGNumberBox
    {
        id: numberBox
        objectName: typeof itemData.indexPath == "undefined" ? "angle_number_box" : itemData.indexPath + "_numberBox"

        width: defaultSpacing.minimumRowHeight * 3

        number: control.angleValue
        minimumValue : itemData.minValue
        maximumValue : itemData.maxValue

        stepSize: itemData.stepSize
        decimals: itemData.decimals
        readOnly: itemData.readOnly
        enabled: itemData.enabled

        suffix: "°"

        multipleValues: itemData.multipleValues

        onEditingFinished:
        {
            angleDial.value = value
        }
        }

    expandedComponent: WGDial
    {
        id: angleDial
        objectName: typeof itemData.indexPath == "undefined" ? "angle_number_box" : itemData.indexPath + "_dial"

        width: defaultSpacing.minimumRowHeight * 3
        height: defaultSpacing.minimumRowHeight * 3

        minimumValue: itemData.minValue
        maximumValue: itemData.maxValue

        // Set the zeroValue so the dial starts with zero to the right
        zeroValue: itemData.minValue + 90

        stepSize: itemData.stepSize
        decimals: itemData.decimals

        value: control.angleValue

        onValueChanged: 
        {
            if (numberBox.number != value)
            {
                setValueHelper( numberBox, "number", value );
            }
            setValueHelper( control, "angleValue", value );
        }
    }
}
