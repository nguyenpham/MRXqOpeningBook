import QtQuick 2.9
import QtQuick.Controls 1.4
import QtQuick.Window 2.3
import QtQuick.Controls.Styles 1.4
import QtQml.Models 2.3

import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import Qt.labs.settings 1.0

import OpeningTreeViewModel 1.0


Window{
    id: myWindow
    title: "Opening"
    width: 800
    height: 600
    visible: true
    flags: Qt.SubWindow

    OpeningModel {
        id: openingModel
    }

    Settings {
        id: winsettings
        category: "window"
        property alias x: myWindow.x
        property alias y: myWindow.y
        property alias width: myWindow.width
        property alias height: myWindow.height
        property alias white: whiteRadioButton.checked
        property alias black: blackRadioButton.checked
    }

    ColumnLayout {
        anchors.rightMargin: 4
        anchors.leftMargin: 4
        anchors.bottomMargin: 4
        anchors.topMargin: 4
        anchors.fill: parent

        PathInput {
            id: openingPath
            forFile: true
            settingsCategory: "showopeningfile"
            Layout.fillWidth: true
            onPathModified: loadOpening()
        }

        // Sides
        RowLayout {
            Label {
                id: forSidesText
                width: 200
                text: qsTr("For sides:")
                Layout.preferredHeight: 20
                Layout.preferredWidth: 80
                font.pixelSize: 12
            }

            GroupBox {
                RowLayout {
                    ExclusiveGroup { id: sideGroup }
                    RadioButton {
                        id: whiteRadioButton
                        text: qsTr("White")
                        checked: true
                        onCheckedChanged: loadOpening()
                        exclusiveGroup: sideGroup
                    }

                    RadioButton {
                        id: blackRadioButton
                        text: qsTr("Black")
                        exclusiveGroup: sideGroup
                    }
                }
            }
        }


        SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            TreeView {
                id: treeView
                backgroundVisible: true
                alternatingRowColors: false
                headerVisible: false
                Layout.fillHeight: true
                Layout.fillWidth: true

                onExpanded: doExpandNode(index, false)

                onCollapsed: {
                    console.log("onCollapsed expanded row:" + index.row)
                    if (working) {
                        return
                    }

                    working = true
                    openingModel.removeExpandedItems(index)
                    working = false
                }

                selection: ItemSelectionModel {
                    model: openingModel
                    onSelectionChanged: doExpandNode(selectedIndexes[0], true)
                }

//                Connections {
//                    target: treeView.selection
//                    onCurrentIndexChanged: {
//                        console.log("onCurrentIndexChanged row: " + treeView.selection.currentIndex.row + ", index = " + index.row)
//                    }
//                    onSelectionChanged: {
//                        console.log("onSelectionChanged Row: " + treeView.currentIndex.row + " Parent : " + treeView.currentIndex.parent.row)
//                    }
//                }

                model: openingModel

                TableViewColumn {
                    role: "title"
                    title: "Title"
                }

                itemDelegate: Item {
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        color: styleData.value.indexOf("@") >= 0 ? "blue" : styleData.value.indexOf("/") >= 0 ? "black" : "gray" //styleData.textColor
                        elide: styleData.elideMode
                        text: styleData.value.replace("@", "")
                    }
                }
            }

            ColumnLayout {
                Board {
                    id: theboard
                    Layout.preferredWidth: 300
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                Button {
                    id: collapseAllBtn
                    text: qsTr("Collapse all")
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    onClicked: {
                        for(var i = 0; i < openingModel.rowCount(); i++) {
                            var idx = openingModel.index(i, 0)
                            treeView.collapse(idx)
                        }
                    }
                }
            }
        }
    }


    property bool working: false

    function doExpandNode(index, selected) {
        if (working) {
            return
        }
        console.log("doExpandNode index:" + index)

        working = true
        treeView.collapse(index);
        openingModel.expand(index)
        if (selected) {
            var moveList = openingModel.selectionChanged(index);
            theboard.updateLine(moveList)
        }
        treeView.expand(index);

        working = false
    }

    Component.onCompleted: {
        loadOpening()
    }

    property string currentPath: ""
    property int currentSd: -1

    function loadOpening() {
        var sd = whiteRadioButton.checked ? 1 : 0;
        if (openingPath.path.length == 0 || (currentPath === openingPath.path.length && sd === currentSd)) {
            return
        }

        currentPath = openingPath.path
        currentSd = sd
        openingModel.loadOpening(openingPath.path, sd)
    }
}
