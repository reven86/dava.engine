import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0

ApplicationWindow {
    id: applicationWindow
    visible: true
    width: 400
    height: 400
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

    Component {
        id: loaderDelegate
        Loader {
            sourceComponent: type == "checkbox" ? checkboxDelegate : radioDelegate;
            property variant modelData: listModel_localOptions.get(index);
            property int index : index;
        }
    }
    Exclus
    iveGroup {
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

    ColumnLayout {
        id: columnLayout_main
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
        anchors.topMargin: 10
        anchors.fill: parent

        RowLayout {
            id: rowLayout_buldFolder
            width: 494
            height: 47

            Label {
                id: label_buildFolder
                text: qsTr("Build folder")
            }

            TextField {
                id: textField_buildFolder
                Layout.fillWidth: true
                placeholderText: qsTr("Text Field")
            }

            Button {
                id: button_getBuildFolder
                iconSource: "qrc:///Resources/openfolder.png"
            }

            Label {
                id: label_buildFolderStatus
                width: 19
                height: 13
                text: qsTr("")
            }
        }

        RowLayout {
            id: rowLayout_davaFolder
            width: 494
            height: 53

            Label {
                id: label_DAVAFolder
                text: qsTr("DAVA folder")
            }

            TextField {
                id: textField_DAVAFolder
                Layout.fillWidth: true
                placeholderText: qsTr("Text Field")
            }

            Button {
                id: button_getDAVAFolder
                iconSource: "qrc:///Resources/openfolder.png"
            }

            Label {
                id: label_DAVAFolderStatus
                width: 19
                height: 13
                text: qsTr("")
            }
        }
        Item
        {
            id: wrapperItem
            Layout.fillHeight: true;
            Layout.fillWidth: true;
            Layout.minimumHeight: {
                label_platforms.height + Math.max(listView_platforms.contentHeight, listView_localOptions.contentHeight)
                        + label_globalOptions.height + gridView_globalOptions.contentHeight
            }
            Layout.minimumWidth: {
                listView_platforms.contentWidth + listView_localOptions.contentWidth
            }

            RowLayout {
                id: rowLayout_platformsAndOptions
                height: parent.height / 3 * 2 -5
                anchors.top: parent.top
                anchors.topMargin: 0
                anchors.right: parent.right
                anchors.rightMargin: 0
                anchors.left: parent.left
                anchors.leftMargin: 0

                ColumnLayout {
                    id: columnLayout_platforms
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 0
                    anchors.top: parent.top
                    anchors.topMargin: 0
                    anchors.right: parent.horizontalCenter
                    anchors.rightMargin: 5
                    anchors.left: parent.left
                    anchors.leftMargin: 0

                    Label {
                        id: label_platforms
                        text: qsTr("Platforms")
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                        anchors.top: parent.top
                        anchors.topMargin: 0
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

                        anchors.top: label_platforms.bottom
                        anchors.topMargin: 5
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 0
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        spacing: 10
                    }
                }

                ColumnLayout {
                    id: columnLayout_localOptions
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 0
                    anchors.top: parent.top
                    anchors.topMargin: 0
                    anchors.left: parent.horizontalCenter
                    anchors.leftMargin: 5
                    anchors.right: parent.right
                    anchors.rightMargin: 0

                    Label {
                        id: label_options
                        text: qsTr("Options")
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                        anchors.top: parent.top
                        anchors.topMargin: 0
                    }

                    ListView {
                        id: listView_localOptions
                        orientation: Qt.Vertical
                        model: listModel_localOptions
                        delegate: loaderDelegate

                        anchors.top: label_options.bottom
                        anchors.topMargin: 5
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 0
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        spacing: 10
                    }
                }
            }

            ColumnLayout {
                id: columnLayout_globalOptions
                height: (parent.height / 3) - 5
                anchors.left: parent.left
                anchors.leftMargin: 0
                anchors.right: parent.right
                anchors.rightMargin: 0
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0

                Label {
                    id: label_globalOptions
                    text: qsTr("global options")
                }

                GridView {
                    id: gridView_globalOptions
                    Layout.fillWidth: true;
                    width: 100
                    height: 100
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 0
                    anchors.top: label_globalOptions.bottom
                    anchors.topMargin: 5

                    delegate: CheckBox {
                        text: model.name
                        checked: model.checked
                        onCheckedChanged: mainObject[globalOptionsOptionName][index]["checked"] = checked;
                    }

                    model: listModel_globalOptions
                }
            }
        }

        RowLayout {
            id: rowLayout_output
            width: 395
            height: 100

            Label {
                id: label_output
                text: qsTr("Output:")
            }

            TextField {
                id: textField_output
                Layout.fillWidth: true;
                placeholderText: qsTr("Text Field")
            }

            Button {
                id: button_runCmake
                text: qsTr("run cmake")
            }
        }
    }

}

