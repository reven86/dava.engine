import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import WGControls 2.0

WGTreeView
{
  property var title: "Property Panel"
  property var layoutHints: { 'dockProperties': 1.0 }

  id: propertyTreeView

  ComponentExtension {
    id: componentExtension
  }

  extensions: [componentExtension]

  property Component propertyDelegate: Loader {
    width: itemWidth
    height: 22
    sourceComponent: itemData.component
  }

  model:PropertyTree

  columnDelegates: [columnDelegate, propertyDelegate]
  columnSequence: [0, 0]
  columnSpacing: 1
}
