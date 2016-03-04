import QtQuick 2.2
import QtQuick.Controls 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import Cpp.Utils 1.0
import Qt.labs.settings 1.0

Item {
    function processMainObject() {
        var arrayPlatforms = mainObject["platforms"];
        if(arrayPlatforms && Array.isArray(arrayPlatforms)) {
            for(var i = 0, length = arrayPlatforms.length; i < length; ++i) {
                listModel_platforms.append(arrayPlatforms[i]);
            }
        }
    }

    signal dataUpdated();

    Layout.minimumHeight: label_platforms.height + listView_platforms.contentHeight + columnLayout_platforms.spacing

    RowLayout {
        id: rowLayout
        anchors.fill: parent
        ListModel {
            id: listModel_platforms
        }
        ListModel {
            id: listModel_localOptions
        }

        ColumnLayout {
            id: columnLayout_platforms
            width: rowLayout.width / 2
            Label {
                id: label_platforms
                text: qsTr("Platforms")
            }

            ListView {
                id: listView_platforms
                orientation: Qt.Vertical
                boundsBehavior: Flickable.StopAtBounds
                model: listModel_platforms
                Layout.fillHeight: true
                spacing: 10
                ExclusiveGroup {
                    id: exclusiveGroup_platforms
                }
                delegate: RadioButton {
                    text: model.name
                    exclusiveGroup: exclusiveGroup_platforms
                    onCheckedChanged: {
                        if(checked) {
                            mainObject["currentPlatform"] = index;
                            mainObject["currentOptions"] = [];
                            listModel_localOptions.clear();
                            var localObject = mainObject["platforms"][index];
                            var options = JSON.parse(JSON.stringify(localObject["options"])); //make a copy
                            if(options && Array.isArray(options)) {
                                for(var i = 0, length = options.length; i < length; ++i) {
                                    options[i]["parentIndex"] = index
                                    listModel_localOptions.append(options[i]);
                                }
                            }
                            dataUpdated();
                        }
                    }
                    Component.onCompleted: if(platformSettings.platformIndex === index) {
                                               checked = true;
                                           }
                }
                Settings {
                    id: platformSettings
                    property int platformIndex;
                }
                Component.onDestruction: platformSettings.platformIndex = mainObject["currentPlatform"];
            }
        }

        ColumnLayout {
            id: columnLayout_localOptions
            width: rowLayout.width / 2
            visible: listModel_localOptions.count !== 0
            Label {
                id: label_options
                text: qsTr("Options")
            }

            ListView {
                id: listView_localOptions
                orientation: Qt.Vertical
                boundsBehavior: Flickable.StopAtBounds
                Layout.fillHeight: true
                model: listModel_localOptions
                delegate: loaderDelegate
                onContentWidthChanged: console.log(contentWidth)
                spacing: 10
                function processDataChanged(checked, value) { //private function
                    var options = mainObject["currentOptions"];
                    if(checked) {
                        options.push(value)
                    } else {
                        for(var i = 0, count = options.length; i < count; i++) {
                            if(options[i].index === value.index) {
                                options.splice(i, 1);
                                break;
                            }
                        }
                    }
                    dataUpdated();
                }
                Component {
                    id: loaderDelegate
                    Loader {
                        sourceComponent: type == "checkbox" ? checkboxDelegate : radioDelegate;
                        property variant modelData: listModel_localOptions.get(model.index);
                        property int index : model.index;
                        Component.onCompleted: {
                            var options = optionsSettings.checkedObjects;

                            if(options && Array.isArray(options)) {
                                for(var i = 0, count = options.length; i < count; i++) {
                                    var option = options[i];
                                    if(option.index === modelData["substring number"] - 1 && option.value === modelData.value) {
                                        item.checked = true
                                    }
                                }
                            }
                        }
                    }
                }
                ExclusiveGroup {
                    id: exclusiveGroup_localOptions
                }

                Component {
                    id: radioDelegate
                    RadioButton {
                        text: modelData ? modelData.name : ""
                        onCheckedChanged: {
                            listView_localOptions.processDataChanged(checked, {"index": modelData["substring number"] - 1, "value": modelData.value});
                        }
                        exclusiveGroup: exclusiveGroup_localOptions
                    }
                }
                Component {
                    id: checkboxDelegate
                    CheckBox {
                        text: modelData ? modelData.name : ""
                        onCheckedChanged: {
                            listView_localOptions.processDataChanged(checked, {"index": modelData["substring number"] - 1, "value": modelData.value});
                        }
                    }
                }
                Settings {
                    id: optionsSettings
                    property var checkedObjects;
                }
                Component.onDestruction: {
                    optionsSettings.checkedObjects = mainObject["currentOptions"];
                }
            }
        }
    }
}
