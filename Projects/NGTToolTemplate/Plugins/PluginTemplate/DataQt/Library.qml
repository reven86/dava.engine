import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import WGControls 1.0

Rectangle {
	color: palette.MainWindowColor
	property var title: "File System"
	property var layoutHints: { 'Library': 1.0 }
	property var sourceContext: source

	WGTreeModel {
		id: libraryModel
		source: sourceContext.GetFileSystemModel()

		ValueExtension {}
		ColumnExtension {}
		ComponentExtension {}
		TreeExtension {}
		ThumbnailExtension {}
		SelectionExtension {
			id: libraryModelSelection
			multiSelect: false
			onSelectionChanged: {
				sourceContext.OnSelectionChanged(getSelection())
			}
		}
	}

	WGTreeView {
		id: libraryView
		anchors.top:parent.top
		anchors.left:parent.left
		anchors.right:parent.right
		anchors.bottom:loadScene.top
		model: libraryModel
		columnDelegates: [defaultColumnDelegate, propertyDelegate]
		selectionExtension: libraryModelSelection
		indentation: 4
		spacing: 1

		property Component propertyDelegate: Loader {
			clip: true
			sourceComponent: itemData != null ? itemData.Component : null
		}
	}

	WGPushButton {
		id: loadScene
		anchors.left:  parent.left
		anchors.leftMargin: 3
		anchors.right: parent.right
		anchors.rightMargin: 3
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 3
		enabled:source.canBeLoaded
		text: "Load Scene"
		onClicked:{
			sourceContext.OnOpenSceneButton();
		}
	}
}
