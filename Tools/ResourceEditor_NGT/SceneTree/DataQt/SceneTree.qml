import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import WGControls 1.0

Rectangle {
  color: palette.MainWindowColor
  property var title: "Scene Tree"
  property var layoutHints: { 'left': 0.5 }

  WGTreeModel
  {
    id: sceneTreeModel
    source: sceneModel

    ValueExtension {}
    ColumnExtension {}
    ComponentExtension {}
    TreeExtension {}
    ThumbnailExtension {}
    SelectionExtension
    {
      id: treeModelSelection
      multiSelect: false
      onSelectionChanged:
      {
        OnSelectionChanged(getSelection())
      }
    }
  }

  WGTreeView
  {
    id: sceneTreeView
    anchors.top:parent.top
    anchors.left:parent.left
    anchors.right:parent.right
    anchors.bottom:parent.bottom
    model: sceneTreeModel
    columnDelegates: [defaultColumnDelegate, propertyDelegate]
    selectionExtension: treeModelSelection
    indentation: 4
    spacing: 1

    property Component propertyDelegate: Loader {
      clip: true
      sourceComponent: itemData != null ? itemData.Component : null
    }
  }
}
