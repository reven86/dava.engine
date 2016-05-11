import QtQuick 2.2
import QtQuick.Controls 1.3

ComboBox {
    id: comboBox_buildFolder
    editable: true

    property alias text: comboBox_buildFolder.editText
    model: ListModel {
        id: listModel
    }
    function addString(text) {
        var obj = {"text" : text};
        listModel.append({"text" : text});
        currentIndex = listModel.count - 1
    }
}
