import QtQuick 2.0

Item {
    id: root
    property string letter: " "
    property int mark: 0

    Text {
        id: markText
        enabled: letter != " "
        anchors.fill: parent
        font.family: xqFont.name
        color: "#00bb00"
        font.pixelSize: (mark == 2 ? 1.0 : 0.4) * root.width
        text: mark == 0 ? " " : "B"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        Text {
            id: bkText
            enabled: letter != ""
            anchors.fill: parent
            font.family: xqFont.name
            color: "white"
            font.pixelSize: 0.82 * root.width
            text: letter == "" ? " " : "B"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            id: pieceText
            enabled: letter != ""
            anchors.fill: parent
            font.family: xqFont.name
            color: letter.charAt() < 'a' ? "black" : "red"

            font.pixelSize: 0.85 * root.width
            text: letter
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}

