import QtQuick 2.9
import QtQuick.Controls 1.4
import QtQuick.Window 2.3

import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0

Window {
    id: xqBoard
    title: "Board"
    width: 800
    height: 600
    visible: true
    flags: Qt.SubWindow

    ColumnLayout {
        anchors.rightMargin: 4
        anchors.leftMargin: 4
        anchors.bottomMargin: 4
        anchors.topMargin: 4
        anchors.fill: parent

        RowLayout {
            Label {
                id: label1
                width: 200
                text: qsTr("Board:")
                font.pixelSize: 12
            }
        }

        Board {
            id: board
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }
}
