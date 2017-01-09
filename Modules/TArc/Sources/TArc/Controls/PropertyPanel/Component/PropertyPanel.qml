import QtQuick 2.4
import QtQuick.Controls 1.4 as QtControls
import TArcControls 1.0

Rectangle
{
    color:"red"
    TreeView
    {
        x: parent.x - 1
        y: parent.y - 1
        width: parent.width + 2
        height: parent.height + 2
        id: propertyPanel
        alternatingRowColors: true
            
        QtControls.TableViewColumn
        {
            id: propertyColumn
            title:"Property"
            role: "display"
            delegate: Item
            {
                Text
                {
                    renderType: Text.NativeRendering
                    anchors.fill: parent
                    text:
                    {
                        if (styleData.value != null && typeof(styleData.value.name) != "undefined")
                            return styleData.value.name;
                        else
                            return "";
                    }
                    elide: Text.ElideMiddle
                }
            }
        }
        
        QtControls.TableViewColumn
        {
            id: valueColumn
            title:"Value"
            role: "display"
            width: propertyPanel.width - propertyColumn.width + 10
            delegate: Item
            {
                Rectangle
                {
                    color:"transparent"
                    x:parent.x
                    y:parent.y
                    width:parent.width - 10
                    height:parent.height
                    Loader
                    {
                        id: componentLoader
                        anchors.fill:parent
                        sourceComponent: styleData.value != null ? styleData.value.component : null
                        Binding
                        {
                            target: componentLoader.item
                            property: "dataContext"
                            value: styleData.value != null ? styleData.value.model : null
                            when: componentLoader.status == Loader.Ready
                        }
                    }
                }
            }
        }

        model: context.model
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
    }
}
