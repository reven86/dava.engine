import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import WGControls 1.0

Rectangle {
  color: palette.MainWindowColor
  property var title: "PropertyPanel"
  property var layoutHints: { 'PropertyPanel': 1.0 }
  property var sourceModel: source

  Label {
    id: searchBoxLabel
    x: reflectedTreeView.leftMargin
    y: 4
    text: "Search:"
  }

  WGTextBox {
    id: searchBox
    y: 2
    anchors.left: searchBoxLabel.right
    anchors.right: parent.right
  }

 WGFilteredTreeModel {
    id: reflectedModel
    source: sourceModel.PropertyTree

    filter: WGTokenizedStringFilter {
      id: stringFilter
      filterText: searchBox.text
      splitterChar: " "
    }

    ValueExtension {}
    ColumnExtension {}
    ComponentExtension {}
    TreeExtension {}
    ThumbnailExtension {}
    SelectionExtension {
      id: treeModelSelection
      multiSelect: false
    }
  }

  WGTreeView {
    id: reflectedTreeView
    anchors.top: searchBox.bottom
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
      sourceComponent: itemData != null ? itemData.Component : null
    }
  }
}
