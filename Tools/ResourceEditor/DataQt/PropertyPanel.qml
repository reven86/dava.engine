import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import WGControls 1.0 as WG1
import WGControls 2.0

WG1.WGPanel
{
    property var title: "Property Panel"
    property var layoutHints: { 'dockProperties': 1.0 }
    color: palette.mainWindowColor

    WGPropertyTreeView
    {
      id: propertyTreeView
      anchors.fill: parent

      model:PropertyTree
    }
}