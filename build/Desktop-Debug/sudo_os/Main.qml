import QtQuick

Window {

    id: rootWindow

    visible: true
    visibility: Window.FullScreen

    Loader {

        id: splashScreenLoader

        source: "screens/Splash.qml"

        anchors.fill: parent

        // Load splashscreen.
        onLoaded: {

            if (item) {

                item.finished.connect(function() {

                    splashScreenLoader.source = "screens/Desktop.qml"

                })

            }

        }

    }

}
