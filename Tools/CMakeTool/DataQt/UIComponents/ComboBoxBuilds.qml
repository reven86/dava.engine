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
        Component.onCompleted: {
            if(modelArray && Array.isArray(modelArray)) {
                for(var i = 0, count = modelArray.length; i < count; i++) {
                    var obj = {"text" : modelArray[i]};
                    listModel.append(obj);
                }
            }
        }
        Component.onDestruction: {
            modelArray = [];
            for(var i = 0, count = listModel.count; i < count; i++) {
                modelArray.push(listModel.get(i).text);
            }
        }
    }
    onEditTextChanged: {
        if(fileSystemHelper.IsDirExists(editText)) {
            var newItem = editText;
            for(var i = model.count - 1; i >= 0; --i) {
                var item = model.get(i).text;
                item = fileSystemHelper.NormalizePath(item);
                var normalizedNewItem = fileSystemHelper.NormalizePath(newItem);
                if(item === normalizedNewItem) {
                    currentIndex = i;
                    return;
                }
            }
            model.insert(0, {"text": editText});

            if(model.count > maxBuildCount) {
                model.remove(maxBuildCount, model.count - maxBuildCount);
            }
        }
    }
}
