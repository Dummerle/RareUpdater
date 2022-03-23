import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12

import "install.js" as Install

Window {
    width: 640
    height: 480
    visible: true
    id: root
    title: qsTr("Rare updater")

    property bool is_installing: false
    property var progress: 0

    Timer  {
        interval: 1000
        running: is_installing
        repeat: true
        onTriggered: () => {
                         progress += 0.1
                         if(progress >= 1){
                             is_installing = false
                         }
                     }
    }

    property var dependencies: [
        "PyQt5",
        "requests",
        "psutil",
        "pypresence",
        "pywin32"
    ]
    property var opt_dependencies: [
        "pywebview[cef]"
    ]

    function on_install(){
        progress = 0
        root.is_installing = true
    }

    Column{
        Text {
            id: title
            text: qsTr("Welcome to Rare updater")
            font.pixelSize: 20
        }

        Text {
            text: qsTr("Select version of Rare to install")
        }

        ComboBox {
            model: ["1.8.8", "1.8.7", "1.7.0"]
        }

        Text {
            id: select_info
            text: qsTr("Select components to install")
        }

        Repeater {
            model: opt_dependencies
            CheckBox {
                id: index
                text: qsTr(modelData)
            }
        }

        ProgressBar {
            id: prog_bar
            visible: is_installing
            value: progress
        }

        Button {
            id: install_button
            text: qsTr("Install")
            onClicked: root.on_install()

        }

    }

}
