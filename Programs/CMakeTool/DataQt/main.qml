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
    property int historyVersion: 4
    property string davaFolderName: "dava.framework";
    objectName: "applicationWindow"
    minimumHeight: wrapper.Layout.minimumHeight + splitView.anchors.margins * 2 + wrapper.spacing * 4
    minimumWidth: wrapper.width + splitView.anchors.margins * 2 + 1
    menuBar: MenuBar {
        Menu {
            title: "CMake Tool"
            MenuItem {
                text: "Show help"
                shortcut: StandardKey.HelpContents
                onTriggered: help.Show();
            }
            MenuItem {
                text: "Preferences"
                shortcut: StandardKey.Preferences
                onTriggered: preferencesDialog.open();
            }
        }
    }
    function processText(text) {
        return text.replace(/(\r\n|\r|\n)+$/g, "").replace(/(\r\n|\r|\n)+/g, displayHtmlFormat ? "<br>" : "\n");
    }
    property bool displayHtmlFormat: true
    Settings {
        id: settings
        property int mainWrapperWidth: 400
        property alias x: applicationWindow.x
        property alias y: applicationWindow.y
        property alias width: applicationWindow.width
        property alias height: applicationWindow.height
        property string historyStr;
        property var lastUsedSourceFolder;
        Component.onDestruction: {
            function compare(left, right) {
                return left.source.localeCompare(right.source);
            }
            history.sort(compare);
            historyStr = JSON.stringify(history)
            historyVersion = applicationWindow.historyVersion;
        }
        property int historyVersion: -1
        property var generateBuildFolderName: false
    }
    property var history;
    function applyProjectSettings(buildSettings) {
        columnLayoutOutput.needClean = buildSettings.needClean;
        rowLayout_buildFolder.path = buildSettings.buildFolder;
        rowLayout_cmakeFolder.path = buildSettings.cmakePath;
        rowLayout_davaFolder.path = buildSettings.davaPath;
        textField_customOptions.text = buildSettings.customOptions
        mutableContent.loadState(buildSettings.state);
    }

    function loadHistory() {
        history = [];

        if(settings.historyVersion === historyVersion) {
            history = JSON.parse(settings.historyStr);
        }

        for(var i = 0, length = history.length; i < length; ++i) {
            rowLayout_sourceFolder.item.addString(history[i].source)
        }
    }

    function onCurrentProjectChaged(index) {
        if(history && Array.isArray(history) && history.length > index) {
            var historyItem = history[index];
            applyProjectSettings(historyItem)
        }
    }

    function addProjectToHistory() {
        var found = false;
        var source = rowLayout_sourceFolder.path;
        settings.lastUsedSourceFolder = source;
        source = fileSystemHelper.NormalizePath(source);
        
        var newItem = {};
        newItem.source = source
        newItem.needClean = columnLayoutOutput.needClean
        newItem.buildFolder = rowLayout_buildFolder.path
        newItem.cmakePath = rowLayout_cmakeFolder.path
        newItem.davaPath = rowLayout_davaFolder.path
        newItem.customOptions = textField_customOptions.text
        newItem.state = mutableContent.saveState();
        
        //now update current history, because we load fields from it.
        for(var i = 0, length = history.length; i < length && !found; ++i) {
            if(history[i].source === source) {
                found = true;
                history[i] = newItem;
            }
        }
        
        //add to combobox and to history
        if(!found) {
            history.push(newItem)
            rowLayout_sourceFolder.item.addString(source)
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

    Help {
        id: help;
    }
    
    Dialog {
        id: preferencesDialog
        visible: false
        title: "CMakeTool preferences dialog"
        standardButtons: StandardButton.Save | StandardButton.Cancel
        onAccepted: {   
            settings.generateBuildFolderName = checkBox_generateBuildFolderName.checked
        }
        onVisibleChanged: { 
            if(visible) {
                checkBox_generateBuildFolderName.checked = settings.generateBuildFolderName
            }
        }
        Column {
            CheckBox {
                id: checkBox_generateBuildFolderName
                text: "generate build folder name"
            }
        }
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
            var sourcePath = fileSystemHelper.NormalizePath(rowLayout_sourceFolder.path)
            var buildPath = fileSystemHelper.NormalizePath(rowLayout_buildFolder.path)
            var cmakePath = fileSystemHelper.NormalizePath(rowLayout_cmakeFolder.path)
            var davaPath = fileSystemHelper.NormalizePath(rowLayout_davaFolder.path)
            var customOptions = textField_customOptions.text
            try {
                var outputText = JSTools.createOutput(configuration,
                                                      fileSystemHelper,
                                                      sourcePath,
                                                      buildPath,
                                                      cmakePath,
                                                      davaPath,
                                                      customOptions);
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
            var lastSource = settings.lastUsedSourceFolder;
            for(var i = 0, length = history.length; i < length; ++i) {
                if(history[i].source === lastSource) {
                    rowLayout_sourceFolder.item.currentIndex = i;
                    onCurrentProjectChaged(i)
                    break;
                }
            }
        }
        catch(error) {
            errorDialog.informativeText = error.message;
            errorDialog.critical = true;
            errorDialog.open();
        }
        applicationWindow.width = Math.max(applicationWindow.width, applicationWindow.minimumWidth)
        applicationWindow.height = Math.max(applicationWindow.height, applicationWindow.minimumHeight)
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
                    id: rowLayout_sourceFolder
                    labelText: qsTr("Source folder");
                    dialogTitle: qsTr("select Source folder");
                    selectFolders: true;
                    inputComponent: ComboBoxBuilds {
                        onTextChanged: {
                            updateOutputString()
                            var found = false;
                            for(var i = 0, length = history.length; i < length && !found; ++i) {
                                if(history[i].source == text) {
                                    found = true;
                                    onCurrentProjectChaged(i);
                                }
                            }
                            if(!found && settings.generateBuildFolderName)
                            {
                                rowLayout_buildFolder.path = text + "/_build";
                            }
                        }
                    }
                }

                RowLayoutPath {
                    id: rowLayout_buildFolder
                    labelText: qsTr("Build folder")
                    dialogTitle: qsTr("select build folder");
                    selectFolders: true;
                    inputComponent: TextField {
                        id: textField_buildFolder
                        placeholderText: qsTr("path to source folder")
                        onTextChanged: {
                            updateOutputString();
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
                        onTextChanged: {
                            updateOutputString();
                        }
                    }
                    Connections {
                        target: rowLayout_sourceFolder
                        onPathChanged: {
                            var path = rowLayout_sourceFolder.path;
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
                    onDataUpdated: {
                        updateOutputString()

                    }
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
                            updateOutputString();
                        }
                    }
                }

                ColumnLayoutOutput {
                    id: columnLayoutOutput
                    Layout.fillWidth: true
                    onCmakeWillBeLaunched: {
                        displayHtmlFormat = true;
                        textArea_processText.text = "";
                        addProjectToHistory();
                    }
                    onCmakeWasLaunched: {
                        //we create build folder
                        rowLayout_buildFolder.refreshPath();
                    }

                    onBuildStarted: {
                        displayHtmlFormat = false;
                        textArea_processText.text = ""
                    }
                }
            }
        }

        TextArea {
            id: textArea_processText
            textFormat: displayHtmlFormat ? TextEdit.RichText : TextEdit.PlainText
            readOnly: true

            Connections {
                target: processWrapper
                onProcessStateChanged: textArea_processText.append(displayHtmlFormat
                                                                   ? "<font color=\"DarkGreen\">" + text + "</font>"
                                                                   : "****new process state: " + text + " ****");
                onProcessErrorChanged: textArea_processText.append(displayHtmlFormat
                                                                   ? "<font color=\"DarkRed\">" + text + "</font>"
                                                                   : "****process error occurred!: " + text + " ****");
                onProcessStandardOutput: {
                    var maxLen = 150;
                    if(!displayHtmlFormat && text.length > maxLen) {
                        textArea_processText.append(processText(text.substring(0, maxLen) + "...\n"));
                    } else {
                        textArea_processText.append(processText(text));
                    }
                }
                onProcessStandardError: textArea_processText.append(displayHtmlFormat
                                                                    ? "<font color=\"DarkRed\">" + processText(text) + "</font>"
                                                                    : processText(text));
            }
        }
    }
}

