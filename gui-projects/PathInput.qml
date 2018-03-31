import QtQuick 2.9
import QtQuick.Controls 1.4
import QtQuick.Window 2.3
import QtQuick.Controls.Styles 1.4
import QtQml.Models 2.3

import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import Qt.labs.settings 1.0

RowLayout {
    id: root
    spacing: 5

    property bool forFile: true
    property string path: ""
    property string settingsCategory: ""

    signal pathModified()

    Settings {
        category: settingsCategory
        property alias opath: pathTextField.text
    }

    //! [filedialog]
    FileDialog {
        id: fileDialog
        visible: false
        modality: Qt.WindowModal
        title: forFile ? qsTr("Choose a file") : qsTr("Choose a folder")
        selectExisting: true
        selectFolder: !forFile
        nameFilters: [ "Opening files (*.xob)", "All files (*)" ]
        selectedNameFilter: "All files (*)"
        onAccepted: {
            pathTextField.text = urlToPath(fileUrl.toString())
        }
    }

    function urlToPath(urlString) {
        var s
        if (urlString.startsWith("file:///")) {
            var k = urlString.charAt(9) === ':' ? 8 : 7
            s = urlString.substring(k)
        } else {
            s = urlString
        }
        return decodeURIComponent(s);
    }

    // Opening path
    Label {
        id: label
        enabled: root.enabled
        width: 200
        text: forFile ? qsTr("Opening file:") : qsTr("Game folder:")
//        font.pixelSize: 12
    }

    TextField {
        id: pathTextField
        enabled: root.enabled
        placeholderText: forFile ? qsTr("Select opening file") : qsTr("Select a folder")
        Layout.fillWidth: true

//        style: TextFieldStyle {
//            textColor: "black"
//            background: Rectangle {
//                radius: 2
//                border.width: 1
//            }
//        }

        onTextChanged: {
            path = text
            pathModified()
        }
    }

    Button {
        id: browserBtn
        enabled: root.enabled
        text: qsTr("Browser...")
        onClicked: fileDialog.open()
    }
}
