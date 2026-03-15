import QtQuick

Rectangle {

    id: splashScreen

    color: windowRoot.bgColor

    anchors.fill: parent

    signal finished

    Image {

        id: logo

        source: windowRoot.splashLogoPath
        antialiasing: true

        property double sizeConstant: 0.4
        width: parent.width * sizeConstant
        height: parent.width * sizeConstant

        anchors.centerIn: parent
        fillMode: Image.PreserveAspectFit

        // Boot animation.
        opacity: 0

        SequentialAnimation {

            id: sequential
            running: true

            PauseAnimation {
                duration: 750
            }

            // Fade in.
            NumberAnimation {

                target: logo
                property: "opacity"
                duration: 1500
                to: 1
                from: 0
            }

            PauseAnimation {
                duration: 1000
            }

            // Fade out.
            NumberAnimation {

                target: logo
                property: "opacity"
                duration: 1500
                to: 0
                from: 1
            }

            onStopped: splashScreen.finished()
        }
    }

    MouseArea {
        anchors.fill: parent

        hoverEnabled: true
        cursorShape: Qt.BlankCursor
        acceptedButtons: Qt.AllButtons
    }
}
