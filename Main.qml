import QtQuick
import QtQuick.Controls

Window {

    id: windowRoot

    // Global Personalization.
    // --- System Constants ---
    readonly property string assetRoot: "qrc:/qt/qml/SudoVM/assets/"
    readonly property string themePath: assetRoot + "textures/personalization/theme/"
    readonly property string componentRoot: "qrc:/qt/qml/SudoVM/elements/"

    // --- Theme Toggle ---
    property bool isDarkMode: true

    // --- Theme Mapping ---
    // These properties react instantly when isDarkMode changes
    readonly property string modeDir: isDarkMode ? "dark" : "light"

    property color bgColor: isDarkMode ? "#101010" : "#fafafa"
    property color panelColor: isDarkMode ? "#eb101010" : "#ebfafafa"

    // --- Personalization ---
    property color accentColor: "#fafafa" // Can be changed to any color via script
    property string currentWallpaper: themePath + modeDir + "/backgrounds/default_" + modeDir + ".png"
    property string splashLogoPath: themePath + modeDir + "/logo/default_" + modeDir + ".png"

    // --- Expandable Logic ---
    // If you want to add "Window Borders" or "Text Colors" easily:
    property color textColor: isDarkMode ? "#fafafa" : "#050505"

    // Main window properties.
    width: 1920
    height: 1080
    visible: true

    color: bgColor

    property int topZIndex: 1

    visibility: Window.FullScreen
    // Comment for fullscreen.
    // flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    title: qsTr("Sudo VM")

    // ------------------------------- //

    // Load Splash.qml to Desktop.qml.
    Loader {
        id: screenLoader
        anchors.fill: parent
        source: "elements/core_screens/Splash.qml"

        // Connect signal from Splash.qml to switch to Desktop.qml
        onLoaded: {
            if (item.finished !== undefined) {
                item.finished.connect(function() {
                    // Switch to Desktop and show Shell.
                    screenLoader.source = "elements/core_screens/Desktop.qml"
                    shell.visible = true
                })
            }
        }
    }

    // Add launch apps logic.
    function spawnApp(title, qmlFile) {
        if (screenLoader.item && screenLoader.item.objectName === "desktop") {
            screenLoader.item.createWindow(title, qmlFile);
        } else {
            console.log("Cannot spawn app: Desktop not loaded");
        }
    }

    // Load Shell.qml and put it on top.
    Item {
        id: shell

        visible: false
        anchors.fill: parent

        z: 100
        clip: true

        Rectangle {
            id: taskBar
            width: 72
            height: 131
            visible: true
            color: windowRoot.panelColor
            border.width: 0
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 0
            bottomRightRadius: 75
            bottomLeftRadius: 15
            topRightRadius: 75
            topLeftRadius: 15

            Image {
                id: openTerminal
                width: 48
                height: 48
                horizontalAlignment: Image.AlignHCenter
                verticalAlignment: Image.AlignVCenter
                source: themePath + modeDir + "/icons/terminal.png"
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit

                MouseArea {
                    id: mouseArea
                    visible: true
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.opacity = 0.7
                    onExited: parent.opacity = 1.0

                    onClicked: {
                        windowRoot.spawnApp("Terminal", componentRoot + "core_apps/Terminal.qml");
                    }
                }
            }
        }
    }
}
