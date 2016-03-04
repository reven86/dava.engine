import QtQuick 2.2
import QtQuick.Controls 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import Cpp.Utils 1.0
import Qt.labs.settings 1.0

RowLayout
{
    property var options: [] //funny rectangle!
    property var mainObject
    onMainObjectChanged:  {
        var arrayPlatforms = mainObject["platforms"];
        if(arrayPlatforms && Array.isArray(arrayPlatforms)) {
            for(var i = 0, length = arrayPlatforms.length; i < length; ++i) {
                listModel_platforms.append(arrayPlatforms[i]);
            }
        }
    }

    ListModel {
        id: listModel_platforms
    }
    ListModel {
        id: listModel_localOptions
    }

    ColumnLayout {
        id: columnLayout_platforms
        height: label_platforms.height + spacing + listView_platforms.contentHeight
        width: Math.max(label_platforms.width, listView_platforms.contentWidth)
        Layout.minimumHeight: label_platforms.height + listView_platforms.contentHeight + spacing
        Layout.minimumWidth: Math.max(label_platforms.width, listView_platforms.width)
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
            ExclusiveGroup {
                id: exclusiveGroup_platforms
            }
            delegate: RadioButton {
                text: model.name
                exclusiveGroup: exclusiveGroup_platforms
                onCheckedChanged: {
                    if(checked) {
                        listModel_localOptions.clear();
                        var localObject = mainObject["platforms"][index];
                        mainObject["currentPlatform"] = index;
                        var options = JSON.parse(JSON.stringify(localObject["options"])); //make a copy
                        if(options && Array.isArray(options)) {
                            for(var i = 0, length = options.length; i < length; ++i) {
                                options[i]["parentIndex"] = index
                                listModel_localOptions.append(options[i]);
                            }
                        }
                    }
                }
            }
            spacing: 10
        }
    }

    ColumnLayout {
        id: columnLayout_localOptions
        visible: listModel_localOptions.count !== 0
        height: label_options.height + spacing + listView_localOptions.contentHeight
        width: Math.max(label_options.width, listView_localOptions.contentWidth)
        Layout.minimumHeight: label_options.height + listView_localOptions.contentHeight + spacing
        Layout.minimumWidth: Math.max(label_options.width, listView_localOptions.width)
        Label {
            id: label_options
            text: qsTr("Options")
        }

        ListView {
            id: listView_localOptions
                Component {
                id: loaderDelegate
                Loader {
                    sourceComponent: type == "checkbox" ? checkboxDelegate : radioDelegate;
                    property variant modelData: listModel_localOptions.get(model.index);
                    property int index : model.index;
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
                        if(!modelData) {
                            return;
                        }
                        if(checked) {
                            options.push(modelData.value)
                        } else {
                            var index = options.indexOf(item);
                            if(index !== -1) {
                                options.splice(index, 1);
                            }
                        }
                    }
                    exclusiveGroup: exclusiveGroup_localOptions
                }
            }
            Component {
                id: checkboxDelegate
                CheckBox {
                    text: modelData ? modelData.name : ""
                    checked: modelData ? modelData.checked : ""
                    onCheckedChanged: {
                        if(!modelData) {
                            return;
                        }
                        updateOutputString();
                    }
                }
            }
            orientation: Qt.Vertical
            boundsBehavior: Flickable.StopAtBounds
            model: listModel_localOptions
            delegate: loaderDelegate
            spacing: 10
        }
    }
}
