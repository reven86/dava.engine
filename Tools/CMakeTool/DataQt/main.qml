import QtQuick 2.2
import QtQuick.Controls 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import Cpp.Utils 1.0
import Qt.labs.settings 1.0
import "UIComponents"
import "Scripts/commandLine.js" as JSTools

ApplicationWindow {
    id: applicationWindow
    visible: true
    width: 800
    height: 600
    property string davaFolderName: "dava.framework";
    objectName: "applicationWindow"
    minimumHeight: wrapper.Layout.minimumHeight + splitView.anchors.margins * 2
    minimumWidth: wrapper.width + splitView.anchors.margins * 2 + 1
    Settings {
        id: settings
        property int mainWrapperWidth: 400
        property alias x: applicationWindow.x
        property alias y: applicationWindow.y
        property alias width: applicationWindow.width
        property alias height: applicationWindow.height
        property alias customOptions: textField_customOptions.text
        property string historyStr;
        Component.onDestruction: historyStr = JSON.stringify(history)
    }
    property var history;
    function applyBuildSettings(buildSettings) {
        columnLayoutOutput.needClean = buildSettings.needClean;
        rowLayout_cmakeFolder.path = buildSettings.cmakePath;
        rowLayout_davaFolder.path = buildSettings.davaPath;
        textField_customOptions.text = buildSettings.customOptions
        mutableContent.loadState(buildSettings.state);
    }

    function loadHistory() {
        try {
            history = JSON.parse(settings.historyStr);
        } catch(e) {
            history = [];
        }

        if(history && Array.isArray(history) && history.length > 0) {
            for(var i = 0, length = history.length; i < length; ++i) {
                rowLayout_buildFolder.item.addString(history[i].build)
            }
            applyBuildSettings(history[0])
        }
    }

    function onCurrentBuildChaged(index) {
        if(history && Array.isArray(history) && history.length > index) {
            applyBuildSettings(history[index])
        }
    }
    property int maxHistoryLength: 10;
    function addBuildToHistory() {
        var found = false;
        var build = rowLayout_buildFolder.path;
        var i = 0;
        for(var length = history.length; i < length && !found; ++i) {
            if(history[i].build === build) {
                found = true;
            }
        }

        if(!found) {
            rowLayout_buildFolder.item.addString(build)
        }

        var newItem = {};
        newItem.build = build
        newItem.needClean = columnLayoutOutput.needClean
        newItem.cmakePath = rowLayout_cmakeFolder.path
        newItem.davaPath = rowLayout_davaFolder.path
        newItem.customOptions = textField_customOptions.text
        newItem.state = mutableContent.saveState();
        history.unshift(newItem);
        if(found) {
            history.splice(i, 1);
        }
        if(history.length > maxHistoryLength) {
            history = history.slice(0, maxHistoryLength);
        }
    }

    title: qsTr("CMake tool")

    ProcessWrapper {
        id: processWrapper;
        Component.onDestruction: BlockingStopAllTasks();
    }

    ConfigStorage {
        id: configStorage
    }

    FileSystemHelper {
        id: fileSystemHelper;
    }

    property var configuration; //main JS object, contained in config file

    Timer {
        id: timer;
        interval: 10
        repeat: false
        onTriggered: updateOutputStringImpl();
    }

    function updateOutputString() {
        timer.start();
    }

    function updateOutputStringImpl() {
        if(configuration) {
            var buildPath = fileSystemHelper.NormalizePath(rowLayout_buildFolder.path)
            var cmakePath = fileSystemHelper.NormalizePath(rowLayout_cmakeFolder.path)
            var davaPath = fileSystemHelper.NormalizePath(rowLayout_davaFolder.path)
            try {
                var outputText = JSTools.createOutput(configuration, fileSystemHelper, buildPath, cmakePath, davaPath);
                columnLayoutOutput.outputComplete = true;
                columnLayoutOutput.outputText = outputText;
            } catch(errorText) {
                columnLayoutOutput.outputComplete = false;
                columnLayoutOutput.outputText = errorText.toString();
            }
        }
    }

    Component.onCompleted: {
        try {
            configuration = JSON.parse(configStorage.GetJSONTextFromConfigFile());
            mutableContent.processConfiguration(configuration);
            loadHistory();
        }
        catch(error) {
            errorDialog.informativeText = error.message;
            errorDialog.critical = true;
            errorDialog.open();
        }
    }

    MessageDialog {
        id: errorDialog;
        text: qsTr("error occurred!")
        icon: StandardIcon.Warning
        property bool critical: false
        onVisibleChanged: {
            if(!visible && critical) {
                Qt.quit()
            }
        }
    }

    SplitView {
        id: splitView;
        anchors.fill: parent
        anchors.margins: 10
        objectName: "splitView"

        Item {
            id: wrapperItem
            width: settings.mainWrapperWidth;
            Component.onDestruction: {
                settings.mainWrapperWidth = width
            }
            Layout.minimumWidth: wrapper.Layout.minimumWidth
            ColumnLayout {
                id: wrapper
                anchors.fill: parent

                RowLayoutPath {
                    id: rowLayout_buildFolder
                    labelText: qsTr("Build folder")
                    dialogTitle: qsTr("select DAVA folder");
                    selectFolders: true;
                    inputComponent: ComboBoxBuilds {
                        id: comboBoxBuilds
                        onTextChanged: {
                            updateOutputString()
                        }
                        onCurrentIndexChanged: onCurrentBuildChaged(currentIndex);
                    }
                }

                RowLayoutPath {
                    id: rowLayout_davaFolder
                    labelText: qsTr("DAVA folder")
                    dialogTitle: qsTr("select DAVA folder");
                    selectFolders: true;
                    inputComponent: TextField {
                        id: textField_davaFolder
                        placeholderText: qsTr("path to dava folder")
                        onTextChanged: {
                            updateOutputString();
                        }
                    }
                    Connections {
                        target: rowLayout_buildFolder
                        onPathChanged: {
                            var path = rowLayout_buildFolder.path;
                            var index = path.indexOf(davaFolderName);
                            if(index !== -1) {
                                path = path.substring(0, index + davaFolderName.length);
                                rowLayout_davaFolder.path = path;
                            }
                        }
                    }
                }

                RowLayoutPath {
                    id: rowLayout_cmakeFolder
                    labelText: qsTr("CMake folder")
                    dialogTitle: qsTr("select CMake executable");
                    selectFolders: false;
                    inputComponent: TextField {
                        id: textField_cmakeFolder
                        placeholderText: qsTr("path to CMake folder")
                        onTextChanged: {
                            var suffix = ".app";
                            if(text.indexOf(suffix, text.length - suffix.length) !== -1) {
                                textField_cmakeFolder.text = text +fileSystemHelper.GetAdditionalCMakePath();
                            }

                            updateOutputString()
                        }
                    }
                    Connections {
                        target: rowLayout_davaFolder

                        onPathChanged: {
                            var path = rowLayout_davaFolder.path;
                            var cmakePath = fileSystemHelper.FindCMakeBin(path, davaFolderName);
                            if(cmakePath.length !== 0) {
                                rowLayout_cmakeFolder.path = cmakePath;
                            }
                        }
                    }
                }

                MutableContentItem {
                    id: mutableContent
                    onDataUpdated: updateOutputString()
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }

                RowLayout {
                    Label {
                        id: label_customOptions
                        text: qsTr("user options")
                    }
                    TextField {
                        id: textField_customOptions
                        Layout.fillWidth: true
                        placeholderText: qsTr("your custom options")
                        onTextChanged: {
                            configuration["customOptions"] = text;
                            updateOutputString();
                        }
                    }
                }

                ColumnLayoutOutput {
                    id: columnLayoutOutput
                    Layout.fillWidth: true
                    onCmakeLaunched: addBuildToHistory();
                }

            }
        }

        TextArea {
            id: textArea_processText
            textFormat: TextEdit.RichText
            readOnly: true
            function toHTML(text) {
                return text.replace(/(\r\n|\r|\n)+$/g, "").replace(/(\r\n|\r|\n)+/g, "<br>");
            }
            Connections {
                target: processWrapper
                onProcessStateChanged: textArea_processText.append("<font color=\"DarkGreen\">" + text + "</font>");
                onProcessErrorChanged: textArea_processText.append("<font color=\"DarkRed\">" + text + "</font>");
                onProcessStandardOutput: textArea_processText.append(textArea_processText.toHTML(text));
                onProcessStandardError: textArea_processText.append("<font color=\"DarkRed\">" + textArea_processText.toHTML(text) + "</font>");
            }
        }
    }
}

