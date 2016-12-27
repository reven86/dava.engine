import QtQuick 2.4
import TArcControls 1.0

TreeView
{
    id: propertyPanel
    alternatingRowColors: true

    model: context.model
    horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

    TableViewColumn
    {
        id: propertyColumn
        title:"Property"
        role: "display"
        delegate: Item
        {
          Text
          {
              anchors.fill: parent
              text: styleData.value.name
              elide: Text.ElideMiddle
          }
        }
    }
    
    TableViewColumn
    {
          id: valueColumn
          title: "Value"
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
                    sourceComponent: styleData.value.component
                    Binding
                    {
                        target: componentLoader.item
                        property: "dataContext"
                        value: styleData.value.model
                        when: componentLoader.status == Loader.Ready
                    }
                }
            }
          }
    }
}
