import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2

ApplicationWindow {
    id: applicationWindow
    visible: true
    width: 400
    height: 350
    title: qsTr("CMake tool")
    signal syncFile();
    ListModel {
        id: listModel_platforms
    }

    ListModel {
        id: listModel_localOptions
    }

    ListModel {
        id: listModel_globalOptions
    }

    property string platfromsOptionName: "platforms";
    property string globalOptionsOptionName: "options";
    property var mainObject;
    signal dataReadyToSave(string text);
    Component.onDestruction: {
        dataReadyToSave(JSON.stringify(mainObject, null, 4));
    }

    Component.onCompleted: {
        try {
            mainObject = JSON.parse(configuration);

            var arrayPlatforms = mainObject[platfromsOptionName];
            if(arrayPlatforms !== undefined && Array.isArray(arrayPlatforms))
            {
                for(var i = 0, length = arrayPlatforms.length; i < length; ++i)
                {
                    listModel_platforms.append(arrayPlatforms[i]);
                }
            }

            var arrayGlobalOptions = mainObject[globalOptionsOptionName];
            if(arrayGlobalOptions !== undefined && Array.isArray(arrayGlobalOptions))
            {
                for(var i = 0, length = arrayGlobalOptions.length; i < length; ++i)
                {
                    listModel_globalOptions.append(arrayGlobalOptions[i]);
                }
            }
        }
        catch(error) {
            errorDialog.informativeText = error.message;
            errorDialog.open();
        }
    }

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

    ListView {
        id: listView_platforms
        orientation: Qt.Vertical
        model: listModel_platforms
        ExclusiveGroup {
            id: exclusiveGroup_platforms
        }
        delegate: RadioButton {
            text: model.name
            checked: model.checked
            exclusiveGroup: exclusiveGroup_platforms
            onCheckedChanged: {
                mainObject[platfromsOptionName][index]["checked"] = checked;
                if(checked)
                {
                    listModel_localOptions.clear();
                    var localObject = mainObject[platfromsOptionName][index];
                    var options = localObject["options"];
                    if(options !== undefined && Array.isArray(options))
                    {
                        for(var i = 0, length = options.length; i < length; ++i)
                        {
                            listModel_localOptions.append(options[i]);
                        }
                    }
                }
            }
        }

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


    Component {
        id: loaderDelegate
        Loader {
            sourceComponent: type == "checkbox" ? checkboxDelegate : radioDelegate;
            property variant modelData: listModel_localOptions.get(index);
            property int index : index;
        }
    }
    ExclusiveGroup {
        id: exclusiveGroup_localOptions
    }
    Component {
        id: radioDelegate
        RadioButton {
            text: modelData ? modelData.name : ""
            checked: modelData ? modelData.checked : false
            onCheckedChanged: mainObject[platfromsOptionName][index]["checked"] = checked;
            exclusiveGroup: exclusiveGroup_localOptions
        }
    }
    Component {
        id: checkboxDelegate
        CheckBox {
            text: modelData ? modelData.name : ""
            onCheckedChanged: mainObject[platfromsOptionName][index]["checked"] = checked;
        }
    }
    ListView {
        id: listView_localOptions
        orientation: Qt.Vertical
        model: listModel_localOptions
        delegate: loaderDelegate

        y: 112
        height: 126
        anchors.verticalCenter: listView_platforms.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.left: listView_platforms.right
        anchors.leftMargin: 6
        spacing: 10
    }

    Label {
        id: label_options
        y: 93
        text: qsTr("Options")
        anchors.left: listView_localOptions.left
        anchors.leftMargin: 0
        anchors.verticalCenterOffset: 0
        anchors.verticalCenter: label_platforms.verticalCenter
    }
    GridView {
        id: gridView_globalOptions

        delegate: CheckBox {
            text: model.name
            checked: model.checked
            onCheckedChanged: mainObject[globalOptionsOptionName][index]["checked"] = checked;
        }

        model: listModel_globalOptions

        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.right: listView_localOptions.right
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
        anchors.top: listView_platforms.bottom
        anchors.topMargin: 15
    }
}

