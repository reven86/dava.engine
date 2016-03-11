import QtQuick 2.2
import QtQuick.Controls 1.3
import Cpp.Utils 1.0
import Qt.labs.settings 1.0

ComboBox {
    id: comboBox_buildFolder
    editable: true
    FileSystemHelper {
        id: fileSystemHelper
    }
    property int maxBuildCount : 10

    property alias text: comboBox_buildFolder.editText
    model: ListModel {
        id: listModel
    }

    Settings {
        id: settings
        property var modelArray;
    }

    Component.onCompleted: {
        blockRebuild = true;
        if(settings.modelArray && Array.isArray(settings.modelArray)) {
            for(var i = 0, count = settings.modelArray.length; i < count; i++) {
                var obj = {"text" : settings.modelArray[i]};
                listModel.append(obj);
            }
        }
        blockRebuild = false;
    }
    property bool blockRebuild: false
    onEditTextChanged: {
        if(fileSystemHelper.IsDirExists(editText)) {
            if(blockRebuild) {
                return;
            }

            var newItem = editText;
            var newObj = {"text": editText};

            var array = settings.modelArray;
            for(var i = model.count - 1; i >= 0; --i) {
                var item = model.get(i).text;
                item = fileSystemHelper.NormalizePath(item);
                var normalizedNewItem = fileSystemHelper.NormalizePath(newItem);
                if(item === normalizedNewItem) {
                    currentIndex = i;
                    array.unshift(item);
                    array.splice(i + 1, 1);
                    settings.modelArray = array;
                    return;
                }
            }
            array.unshift(editText);
            var length = array.length;
            if(length > maxBuildCount) {
                array.splice(maxBuildCount, length - maxBuildCount);
            }
            settings.modelArray = array;

            model.insert(0, newObj);
            if(model.count > maxBuildCount) {
                model.remove(maxBuildCount, model.count - maxBuildCount);
            }
        }
    }
}
