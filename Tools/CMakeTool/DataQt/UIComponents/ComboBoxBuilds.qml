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
        return;
        if(fileSystemHelper.IsDirExists(editText)) {
            var newItem = editText;
            var found = false;
            for(var i = model.count - 1; i >= 0 && !found; --i) {
                var item = model.get(i).text;
                if(item === newItem
                        || item + "/" === newItem
                        || newItem + "/" === item) {
                    found = true;
                    newItem = item;
                    model.remove(i, 1);
                }
            }
            model.insert(0, {"text": editText});

            if(model.count > maxBuildCount) {
                model.remove(maxBuildCount, model.count - maxBuildCount);
            }
        }
    }
}
