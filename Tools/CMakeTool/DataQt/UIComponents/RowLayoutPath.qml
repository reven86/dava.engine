import QtQuick 2.2
import QtQuick.Controls 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import Cpp.Utils 1.0

RowLayout {
    property alias labelText: label.text
    property alias dialogTitle: fileDialog.title
    property bool selectFolders: false;
    property alias inputComponent: loader.sourceComponent
    property string path: loader.item.text
    onPathChanged: loader.item.text = path
    Layout.minimumWidth: label.width + button.width + image.width + spacing * 3 + 50
    Layout.minimumHeight: Math.max(label.height, button.height, image.height, loader.item.height)
    function isPathValid(path) {
        if(selectFolders) {
            return fileSystemHelper.IsDirExists(path);
        } else {
            return fileSystemHelper.IsFileExists(path);
        }
    }

    Label {
        id: label
    }

    Loader {
        id: loader
        Layout.fillWidth: true
    }

    FileSystemHelper {
        id: fileSystemHelper
    }

    FileDialog {
        id: fileDialog
        selectFolder: selectFolders
        onAccepted: {
            var url = fileDialog.fileUrls[0].toString()
            url = fileSystemHelper.ResolveUrl(url);
            loader.item.text = url;
        }
    }

    Button {
        id: button
        iconSource: "qrc:///Icons/openfolder.png"
        onClicked: {
            fileDialog.folder = loader.item.text;
            fileDialog.open();
        }
    }

    Image {
        id: image
        width: height
        height: parent.height
        source: "qrc:///Icons/" + (isPathValid(loader.item.text) ? "ok" : "error") + ".png"
    }
}
