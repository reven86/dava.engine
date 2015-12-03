import QtQuick 2.1

Image
{
    id : slotImage
    source: slot.slotObject.icon
    sourceSize.width : 12
    sourceSize.height : 12

    property var slot

    width : contentRect.width
    height : contentRect.height
    z : parent.z + 10

    function getGlobalCenter()
    {
        return mapToItem(objectsContainer, width / 2.0, height / 2.0)
    }

    Component.onCompleted :
    {
        slotsIndex[slot.slotObject.uid] = slotImage;
    }

    Component.onDestruction :
    {
        delete slotsIndex[slot.slotObject.uid];
    }

    states :
    [
        State
        {
            name : "interactive"
            when : connectionStartSlot != null
            PropertyChanges
            {
                target: slotImage
                source : connectionStartSlot.isInput != slot.isInput ? slot.slotObject.icon : ""
            }
        }
    ]

    MouseArea
    {
        id : mouseArea
        anchors.fill : parent
        acceptedButtons : Qt.LeftButton

        onPressed :
        {
            var pt = getGlobalCenter();
            interactiveConnectionCurve.startX = pt.x;
            interactiveConnectionCurve.startY = pt.y;
            interactiveConnectionCurve.endX = pt.x;
            interactiveConnectionCurve.endY = pt.y;
            interactiveConnectionCurve.visible = true;
            connectionStartSlot = slot
        }

        onPositionChanged :
        {
            var pt = mapToItem(objectsContainer, mouse.x, mouse.y)
            interactiveConnectionCurve.endX = pt.x;
            interactiveConnectionCurve.endY = pt.y;
        }

        function findSlot(root, point)
        {
            for(var i = 0; i < root.visibleChildren.length; ++i)
            {
                var child = root.visibleChildren[i];
                if(child == interactiveConnectionCurve)
                    continue;

                var pt = root.mapToItem(child, point.x, point.y)
                if (pt.x >= 0 && pt.x <= child.width &&
                    pt.y >= 0 && pt.y <= child.height)
                {
                    if (child.objectName === "ConnectionSlot")
                        return child

                    var obj =  findSlot(child, pt);
                    if (obj != null)
                        return obj
                }
            }

            return null
        }

        onReleased :
        {
            connectionStartSlot = null
            interactiveConnectionCurve.startX = 0;
            interactiveConnectionCurve.startY = 0;
            interactiveConnectionCurve.endX = 0;
            interactiveConnectionCurve.endY = 0;
            interactiveConnectionCurve.visible = false;

            var point = mapToItem(objectsContainer, mouse.x, mouse.y)

            var child = findSlot(objectsContainer, point)
            if (child == null)
                return;

            if (slot.isInput)
                graphEditorComponent.connect(slot.slotObject.uid, child.slotObject.uid)
            else
                graphEditorComponent.connect(child.slotObject.uid, slot.slotObject.uid)
        }

        ContextMenu
        {
            menuModel : graphEditorComponent.getSlotContextMenu()
            z : slotImage.z + 10
            contextObjectUid : slot.slotObject.uid
        }
    }
}