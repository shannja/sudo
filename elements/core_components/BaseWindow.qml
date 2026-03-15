import QtQuick

Rectangle {
    id: baseWindow

    property alias appSource: appLoader.source
    property alias windowTitle: title.text

    property bool isFocused: true
    property bool isMaximized: false
    property int def_width: 1152
    property int def_height: 580
    property int curr_width: def_width
    property int curr_height: def_height
    property int curr_x: baseWindow.x
    property int curr_y: baseWindow.y

    width: def_width
    height: def_height
    color: windowRoot.panelColor
    radius: 15
    clip: false

    border.color: isFocused ? windowRoot.accentColor : windowRoot.bgColor

    // --- AUTO-FOCUS ON SPAWN ---
    // This runs as soon as the window is created
    Component.onCompleted: {
        baseWindow.x = (parent.width - baseWindow.width) / 2
        baseWindow.y = (parent.height - baseWindow.height) / 2

        unFocus()
    }

    // CATCHER
    MouseArea {
        anchors.fill: parent
        onPressed: {
            unFocus()
        }
    }

    Rectangle {
        id: titleBar
        height: 35
        color: isFocused ? windowRoot.accentColor : windowRoot.bgColor
        radius: 15
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        bottomLeftRadius: 0
        bottomRightRadius: 0

        MouseArea {
            id: dragArea
            anchors.fill: parent
            drag.target: baseWindow
            onPressed: {
                // Only allow dragging if NOT maximized
                if (!baseWindow.isMaximized) {
                    drag.target = baseWindow
                } else {
                    drag.target = null
                }
                unFocus()
            }
        }

        Text {
            id: title
            // Text flips to bgColor (Dark/Light) when titleBar is Accent
            color: isFocused ? windowRoot.bgColor : windowRoot.textColor
            text: qsTr("Window Title")
            anchors.fill: parent
            anchors.leftMargin: 15
            anchors.rightMargin: 150
            font.pixelSize: 12
            verticalAlignment: Text.AlignVCenter
            font.family: "Verdana"
        }

        Image {
            id: exit
            width: 16
            height: 16
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: 8
            anchors.topMargin: 10
            source: {
                let mode = isFocused ? (windowRoot.isDarkMode ? "light" : "dark") : windowRoot.modeDir
                return windowRoot.themePath + mode + "/icons/quit.png"
            }
            fillMode: Image.PreserveAspectFit
            MouseArea {
                anchors.fill: parent
                onClicked: baseWindow.destroy()
            }
        }

        Image {
            id: toggleMinMax
            x: 594
            width: 16
            height: 16
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: 30
            anchors.topMargin: 10
            // ICON LOGIC:
            // If focused, we force the icon to be the OPPOSITE mode
            // so it's visible against the accent color.
            source: {
                let mode = isFocused ? (windowRoot.isDarkMode ? "light" : "dark") : windowRoot.modeDir
                let icon = isMaximized ? "minimize.png" : "maximize.png"
                return windowRoot.themePath + mode + "/icons/" + icon
            }
            anchors.verticalCenterOffset: 0
            fillMode: Image.PreserveAspectFit
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    fullScreen(); // Just handle the math here
                    unFocus();    // Ensure it grabs focus
                }
            }
        }
    }

    // Resize Handle (Bottom-Right Corner)
    MouseArea {
        id: resizeHandle
        width: 20
        height: 20
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        cursorShape: Qt.SizeFDiagCursor // Changes cursor to the resize diagonal

        // Disable resizing if the window is maximized
        visible: !isMaximized

        property point lastMousePos

        onPressed: {
            lastMousePos = Qt.point(mouseX, mouseY)
            unFocus() // Grab focus when starting resize
        }

        onPositionChanged: {
            if (pressed) {
                let dx = mouseX - lastMousePos.x
                let dy = mouseY - lastMousePos.y

                // Calculate new dimensions
                let newWidth = baseWindow.width + dx
                let newHeight = baseWindow.height + dy

                // Set Minimum Bounds (Prevent window from disappearing)
                if (newWidth > 480) baseWindow.width = newWidth
                if (newHeight > 360) baseWindow.height = newHeight
            }
        }
    }

    Item {
        id: windowSpace
        anchors.top: titleBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clip: true

        Loader {
            id: appLoader
            anchors.fill: parent
        }
    }

    function unFocus() {
        // Find appSpace (the parent container)
        if (parent) {
            var siblings = parent.children;
            for (var i = 0; i < siblings.length; i++) {
                if (siblings[i] !== baseWindow && siblings[i].isFocused !== undefined) {
                    siblings[i].isFocused = false;
                }
            }
        }
        baseWindow.z = ++windowRoot.topZIndex;
        baseWindow.isFocused = true;
    }

    function fullScreen() {
        isMaximized = !isMaximized

        if (isMaximized) {
            curr_height = baseWindow.height
            curr_width = baseWindow.width
            curr_x = baseWindow.x
            curr_y = baseWindow.y

            baseWindow.anchors.fill = parent
        } else {
            baseWindow.anchors.fill = undefined

            baseWindow.width = curr_width
            baseWindow.height = curr_height

            baseWindow.x = curr_x
            baseWindow.y = curr_y
        }
    }
}
