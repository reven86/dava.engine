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
    //minimumWidth: wrapper.Layout.minimumWidth + 100 + splitView.anchors.margins * 2  //100 for output text area
    minimumWidth: wrapper.width + splitView.anchors.margins * 2 + 1
    Settings {
        id: settings
        property int mainWrapperWidth: 400
        property alias x: applicationWindow.x
        property alias y: applicationWindow.y
        property alias width: applicationWindow.width
        property alias height: applicationWindow.height
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

    property bool outputComplete: false
    property var mainObject; //main JS object, contained in config file

    Timer {
        id: timer;
        interval: 10
        repeat: false
        onTriggered: updateOutputStringImpl();
    }

    function extractCMakePathFromPath(path) {
        var cmakePath = fileSystemHelper.FindCMakeBin(textField_DAVAFolder.text);
        if(cmakePath.length !== 0) {
            textField_CMakeFolder.text = cmakePath;
        }
    }

    function updateOutputString() {
        timer.start();
    }

    function updateOutputStringImpl() {
        columnLayoutOutput.outputText = JSTools.createOutput(mainObject, outputComplete);
    }

    Component.onCompleted: {
        try {
            var configuration = configStorage.GetJSONTextFromConfigFile()
            mainObject = JSON.parse(configuration);
            mutableContent.mainObject = mainObject
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
                        onTextChanged: {
                            updateOutputString()
                        }
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
                        Settings {property alias text: textField_davaFolder.text }
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
                                rowLayout_davaFolder.path = path.substring(0, index + davaFolderName.length);
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
                        Settings {property alias text: textField_cmakeFolder.text }
                        onTextChanged: {
                            updateOutputString()
                        }
                    }
                    Connections {
                        target: rowLayout_davaFolder

                        onPathChanged: {
                            var path = rowLayout_davaFolder.path;
                            var cmakePath = fileSystemHelper.FindCMakeBin(path);
                            if(cmakePath.length !== 0) {
                                rowLayout_cmakeFolder.path = cmakePath;
                            }
                        }
                    }
                }

                MutableContentItem {
                    id: mutableContent
                }

                ColumnLayoutOutput {
                    id: columnLayoutOutput
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

