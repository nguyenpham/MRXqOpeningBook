import QtQuick 2.9
import QtQuick.Controls 1.4

import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import Qt.labs.settings 1.0

ApplicationWindow {
    id: applicationWindow
    visible: true
    width: 400
    height: 300
    title: qsTr("Opening GUI")
    minimumWidth: 250

    Settings {
        id: appsettings
        category: "app"
        property alias x: applicationWindow.x
        property alias y: applicationWindow.y
        property alias width: applicationWindow.width
        property alias height: applicationWindow.height
    }

    ColumnLayout {
        anchors.rightMargin: 4
        anchors.leftMargin: 4
        anchors.bottomMargin: 4
        anchors.topMargin: 4
        anchors.fill: parent

        ComboBox {
            id: languageComboBox
            Layout.preferredHeight: 30
            Layout.preferredWidth: 200
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            currentIndex: 0
            model: ListModel {
                id: cbItems
                ListElement { text: "English" }
            }
            onCurrentIndexChanged: console.debug(cbItems.get(currentIndex).text + ", " + cbItems.get(currentIndex).color)
        }

        Button {
            id: builderbtn
            text: qsTr("Build & verify")
            Layout.preferredHeight: 30
            Layout.preferredWidth: 200
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            property QtObject builderWindow: null

            onClicked:{
                if (builderWindow != null) {
                    builderWindow.raise()
                    return;
                }
                var component = Qt.createComponent("Builder.qml");
                if (component.status === Component.Ready) {
                    builderWindow = component.createObject(applicationWindow);
                    builderWindow.visibleChanged.connect(
                                  function() {
                                      if (!builderWindow.visible) {
                                          builderWindow.destroy()
                                          builderWindow = null
                                      }
                                  }
                                  );
                }
            }

        }

        Button {
            id: viewbtn
            Layout.preferredHeight: 30
            Layout.preferredWidth: 200
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            text: qsTr("Opening view / edit")
            property QtObject openingWindow
            onClicked:{
                if (openingWindow != null) {
                    openingWindow.raise()
                    return;
                }
                var component = Qt.createComponent("OpeningView.qml");
                if (component.status === Component.Ready) {
                    openingWindow = component.createObject(applicationWindow);
                    openingWindow.visibleChanged.connect(
                                  function() {
                                      if (!openingWindow.visible) {
                                          openingWindow.destroy()
                                          openingWindow = null
                                      }
                                  }
                                  );
                }
            }
        }

//        Button {
//            id: boardbtn
//            Layout.preferredHeight: 30
//            Layout.preferredWidth: 200
//            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
//            text: qsTr("Test Board")
//            property QtObject boardWindow

//            onClicked:{
//                if (boardWindow != null) {
//                    boardWindow.raise()
//                    return;
//                }

//                var component = Qt.createComponent("BoardView.qml");
//                if (component.status === Component.Ready) {
//                    boardWindow = component.createObject(applicationWindow);
//                    boardWindow.visibleChanged.connect(
//                                  function() {
//                                      if (!boardWindow.visible) {
//                                          boardWindow.destroy()
//                                          boardWindow = null
//                                      }
//                                  }
//                                  );
//                }
//            }
//        }
    }


}
