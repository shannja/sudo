import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {

    id: desktopRoot
    objectName: "desktop"

    color: windowRoot.bgColor
    anchors.fill: parent

    // Fade In all.
    opacity: 0
    NumberAnimation {
        id: imagefadeIn
        target: desktopRoot
        property: "opacity"
        running: true
        duration: 1000
        to: 1
        from: 0
    }

    Image {
        id: wallpaper

        source: windowRoot.currentWallpaper
        antialiasing: true

        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop

        MouseArea {
            id: unfocus
            anchors.fill: parent

            onClicked: {
                // Find all windows in appSpace and unfocus them
                for (var i = 0; i < appSpace.children.length; i++) {
                    appSpace.children[i].isFocused = false
                }
            }
        }
    }

    // ------------------------------- //
    // App space.
    // Will host all applications etc.

    Item {
        id: appSpace
        anchors.fill: parent
    }

    // The logic to spawn windows
    function createWindow(title, qmlFile) {
        var component = Qt.createComponent(windowRoot.componentRoot + "core_components/BaseWindow.qml");
        if (component.status === Component.Ready) {
            // We create the BaseWindow and set its parent to appSpace
            var win = component.createObject(appSpace, {
                "windowTitle": title,
                "appSource": qmlFile
            });
        } else {
            console.error("Error loading BaseWindow:", component.errorString());
        }
    }

    // ------------------------------- //
    // Shell. (Taskbar, etc. persistent on top.)

}
