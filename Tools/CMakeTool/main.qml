import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2

ApplicationWindow {
    id: applicationWindow
    visible: true
    width: 400
    height: 350
    title: qsTr("CMake tool")

    MessageDialog {
        id: errorDialog;
        text: "error occurred!"
        visible: false
        icon: StandardIcon.Warning

    }

    TextField {
        id: textField_buildFolder
        y: 19
        anchors.verticalCenter: label_buildFolder.verticalCenter
        anchors.left: label_buildFolder.right
        anchors.leftMargin: 19
        placeholderText: qsTr("Text Field")
    }

    Label {
        id: label_buildFolder
        text: qsTr("Build folder")
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
    }

    Button {
        id: button_getBuildFolder
        y: 17
        text: qsTr("Button")
        anchors.left: textField_buildFolder.right
        anchors.leftMargin: 6
        anchors.verticalCenter: label_buildFolder.verticalCenter
    }

    TextField {
        id: textField_DAVAFolder
        y: 60
        anchors.verticalCenter: label_DAVAFolder.verticalCenter
        anchors.left: label_DAVAFolder.right
        anchors.leftMargin: 14
        placeholderText: qsTr("Text Field")
    }

    Label {
        id: label_DAVAFolder
        text: qsTr("DAVA folder")
        anchors.top: label_buildFolder.bottom
        anchors.topMargin: 22
        anchors.left: label_buildFolder.left
        anchors.leftMargin: 0
    }

    Button {
        id: button_getDAVAFolder
        y: 59
        text: qsTr("Button")
        anchors.verticalCenter: textField_DAVAFolder.verticalCenter
        anchors.left: textField_DAVAFolder.right
        anchors.leftMargin: 6
    }

    Label {
        id: label_buildFolderStatus
        y: 22
        width: 19
        height: 13
        text: qsTr("")
        anchors.verticalCenter: button_getBuildFolder.verticalCenter
        anchors.left: button_getBuildFolder.right
        anchors.leftMargin: 6
    }

    Label {
        id: label_DAVAFolderStatus
        y: 64
        width: 19
        height: 13
        text: qsTr("")
        anchors.verticalCenter: button_getDAVAFolder.verticalCenter
        anchors.left: button_getDAVAFolder.right
        anchors.leftMargin: 6
    }

    Column {
        id: column_platfroms
        width: 181
        height: 126
        anchors.top: label_platforms.bottom
        anchors.topMargin: 10
        anchors.left: label_platforms.left
        anchors.leftMargin: 0
        spacing: 10
    }

    Label {
        id: label_platforms
        text: qsTr("Platforms")
        anchors.left: label_DAVAFolder.left
        anchors.leftMargin: 0
        anchors.top: label_DAVAFolder.bottom
        anchors.topMargin: 16
    }

    Column {
        id: column_options
        y: 112
        height: 126
        anchors.verticalCenter: column_platfroms.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.left: column_platfroms.right
        anchors.leftMargin: 6
        spacing: 10
    }

    Label {
        id: label_options
        y: 93
        text: qsTr("Options")
        anchors.left: column_options.left
        anchors.leftMargin: 0
        anchors.verticalCenterOffset: 0
        anchors.verticalCenter: label_platforms.verticalCenter
    }
    GridView {
        id: globalOptionsView
        Component.onCompleted: {
            try {
                var object = JSON.parse(configuration);
                var array = object["Options"];
                if(array !== undefined && Array.isArray(array))
                {
                    console.log(array[0]["name"])
                    globalOptionsModel.append(array[0])

                    for(var obj in array)
                    {
                        globalOptionsModel.append(obj)
                    }
                }
            }
            catch(error) {
                errorDialog.informativeText = error.message;
                errorDialog.open();
            }
        }
        delegate: CheckBox {
            text: name
            property string innerValue : innerValue
        }
        model: ListModel {
            id: globalOptionsModel
        }

        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.right: column_options.right
        anchors.rightMargin: 0
        anchors.top: label_globalOptions.bottom
        anchors.topMargin: 6
        anchors.left: label_globalOptions.left
        anchors.leftMargin: 0
    }

    Label {
        id: label_globalOptions
        text: qsTr("global options")
        anchors.left: label_platforms.left
        anchors.leftMargin: 0
        anchors.top: column_platfroms.bottom
        anchors.topMargin: 6
    }
}

