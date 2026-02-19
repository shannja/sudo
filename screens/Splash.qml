import QtQuick


Rectangle {

    id: splashScreen

    color: "#101010"

    anchors.fill: parent

    signal finished()

    Image {

        id: logo

        source: "../textures/logo/sudo_logo.png"

        width: parent.width * 0.4
        height: parent.width * 0.4

        anchors.centerIn: parent
        fillMode: Image.PreserveAspectFit

        opacity: 0 // Start hidden

        SequentialAnimation {

            running: true

            // Pause for a sec.
            PauseAnimation { duration: 750 }

            // 1.5s Fade In.
            NumberAnimation {

                target: logo; property: "opacity"
                from: 0; to: 1; duration: 1500

            }

            // Pause for 1s.
            PauseAnimation { duration: 1000 }

            // 1.5s Fade Out.
            NumberAnimation {

                target: logo; property: "opacity"
                from: 1; to: 0; duration: 1500

            }

            onFinished: splashScreen.finished()

        }

    }

    // Limits the mouse actions.
    MouseArea {

        anchors.fill: parent

        // Show as blank.
        cursorShape: Qt.BlankCursor

        // Consumes all actions.
        hoverEnabled: true
        acceptedButtons: Qt.AllButtons

    }

}
