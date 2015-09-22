import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import BWControls 1.0
import WGControls 1.0

Rectangle {
  color: palette.MainWindowColor
  property var title: "PropertyPanel"
  property var layoutHints: { 'PropertyPanel': 1.0 }
  property var sourceModel: source

 WGTreeModel {
    id: reflectedModel
    source: sourceModel

    ValueExtension {}
    ColumnExtension {}
    ComponentExtension {}
    TreeExtension {}
    ThumbnailExtension {}
    SelectionExtension {
      id: treeModelSelection
      multiSelect: true
    }
  }

  WGTreeView {
    id: reflectedTreeView
    anchors.top: parent.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    model: reflectedModel
    columnDelegates: [defaultColumnDelegate, propertyDelegate]
    selectionExtension: treeModelSelection
    indentation: 4
    spacing: 1

    property Component propertyDelegate: Loader {
      clip: true
      sourceComponent: {
        console.log("Log in qml : ", itemData.Component)
        itemData != null ? itemData.Component : null
      }
    }
  }
}
