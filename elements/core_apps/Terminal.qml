import QtQuick 2.15
import QtQuick.Layouts 2.15
import QtQuick.Controls 2.15
import SudoVM.Core

Item {
    id: terminalRoot

    TerminalBridge {
        id: bridge
    }

    property bool isWaitingForInput: bridge.isWaiting
    property string targetVariable: ""

    // --- THE ZOOM SOURCE OF TRUTH ---
    property int globalFontSize: 13

    ColumnLayout {
        id: column
        anchors.fill: parent
        spacing: 0

        // --- MENU BAR (Buttons) ---
        RowLayout {
            id: menu
            Layout.margins: 15
            spacing: 25

            // RUN BUTTON
            Text {
                id: run
                opacity: mouseAreaRun.containsMouse ? 0.7 : 1.0
                text: qsTr("Run")
                color: windowRoot.accentColor
                font.pixelSize: 12
                font.family: "Verdana"
                font.bold: true

                MouseArea {
                    id: mouseAreaRun
                    anchors.fill: parent
                    hoverEnabled: true
                    onPressed: {
                        let rawInput = inputField.text.trim();
                        if (rawInput !== "" && baseWindow.isFocused) {
                            logModel.append({"line": "> Running command(s)..."});
                            processCommand(rawInput);
                        }
                    }
                }
            }

            // ZOOM IN BUTTON
            Text {
                id: increaseText
                opacity: mouseAreaZoomIn.containsMouse ? 0.7 : 1.0
                color: windowRoot.accentColor
                text: qsTr("Zoom In (+)")
                font.pixelSize: 12
                font.family: "Verdana"

                MouseArea {
                    id: mouseAreaZoomIn
                    anchors.fill: parent
                    hoverEnabled: true
                    onPressed: {
                        if (terminalRoot.globalFontSize < 40)
                            terminalRoot.globalFontSize += 2
                    }
                }
            }

            // ZOOM OUT BUTTON
            Text {
                id: decreaseText
                opacity: mouseAreaZoomOut.containsMouse ? 0.7 : 1.0
                color: windowRoot.accentColor
                text: qsTr("Zoom Out (-)")
                font.pixelSize: 12
                font.family: "Verdana"

                MouseArea {
                    id: mouseAreaZoomOut
                    anchors.fill: parent
                    hoverEnabled: true
                    onPressed: {
                        if (terminalRoot.globalFontSize > 8)
                            terminalRoot.globalFontSize -= 2
                    }
                }
            }
        }

        // --- SEPARATOR LINE ---
        Rectangle {
            id: line
            color: windowRoot.accentColor
            Layout.rightMargin: 15
            Layout.leftMargin: 15
            Layout.maximumWidth: 65530
            Layout.preferredHeight: 1
            Layout.fillWidth: true
        }

        // --- INPUT AREA ---
        ScrollView {
            id: input
            Layout.margins: 15
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AlwaysOff }
            ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AlwaysOff }

            TextArea {
                id: inputField
                placeholderText: "Type command or script..."
                placeholderTextColor: baseWindow.isFocused ? windowRoot.textColor : windowRoot.panelColor
                focus: baseWindow.isFocused
                enabled: !bridge.isWaiting

                font.pixelSize: terminalRoot.globalFontSize
                font.family: "Monospace"

                color: windowRoot.accentColor
                background: null
                wrapMode: TextArea.NoWrap
                tabStopDistance: font.pixelSize * 3

                selectByMouse: true
                selectedTextColor: windowRoot.bgColor
                selectionColor: windowRoot.accentColor

                MouseArea {
                    anchors.fill: parent
                    enabled: !baseWindow.isFocused
                    onPressed: {
                        if (!baseWindow.isFocused) {
                            baseWindow.unfocus()
                        }
                    }
                }

                Keys.onPressed: (event) => {
                    if (event.key === Qt.Key_Return && event.modifiers & Qt.ControlModifier) {
                        let rawInput = inputField.text.trim();
                        if (rawInput !== "") {
                            logModel.append({"line": "> Running..."});
                            processCommand(rawInput);
                        }
                        event.accepted = true;
                    }
                }
            }
        }

        // --- OUTPUT LABEL ---
        Text {
            id: label
            text: qsTr("Output Log")
            color: windowRoot.textColor
            font.pixelSize: 11
            font.bold: true
            Layout.topMargin: 5
            Layout.bottomMargin: 5
            Layout.margins: 15
        }

        // --- OUTPUT AREA ---
        Rectangle {
            id: outputContainer
            color: "transparent"
            radius: 10
            border.color: baseWindow.isFocused ? windowRoot.accentColor : windowRoot.bgColor
            Layout.maximumHeight: 150

            Layout.margins: 15
            Layout.preferredHeight: 150
            Layout.minimumHeight: 100
            Layout.fillHeight: true
            Layout.fillWidth: true

            ListView {
                id: logView
                anchors.fill: parent
                anchors.margins: 10
                interactive: true
                clip: true

                onCountChanged: Qt.callLater(logView.positionViewAtEnd)

                model: ListModel { id: logModel }

                delegate: Item {
                    width: logView.width
                    height: contentRow.height + 4

                    readonly property bool isInput: model.line.startsWith("INPUT_REQUEST:")
                    readonly property bool isHistory: model.line.startsWith("[HISTORY]")
                    readonly property string varName: isInput ? model.line.split(":")[1] : ""

                    RowLayout {
                        id: contentRow
                        width: parent.width
                        spacing: 0

                        Text {
                            id: outputText
                            text: isInput ? "Input " + varName + ": " :
                                  (isHistory ? model.line.replace("[HISTORY]", "") : model.line)

                            color: isInput ? windowRoot.accentColor : windowRoot.textColor
                            font.pixelSize: terminalRoot.globalFontSize
                            font.family: "Monospace"

                            // STYLE LOGIC:
                            // Italicize if it starts with ">" (System Output / Finished message)
                            // Keep normal if it is history (User input) or an active input prompt
                            //font.italic: model.line.startsWith(">")

                            Layout.fillWidth: false
                        }

                        TextField {
                            id: activeInput
                            visible: isInput
                            enabled: isInput && index === (logModel.count - 1)

                            leftPadding: 0
                            rightPadding: 0
                            topPadding: 0
                            bottomPadding: 0

                            font.pixelSize: terminalRoot.globalFontSize
                            font.family: "Monospace"
                            color: windowRoot.accentColor
                            background: null

                            Layout.fillWidth: true
                            focus: visible

                            onAccepted: {
                                if (text.trim() !== "") {
                                    bridge.sendCommand(text);
                                }
                            }
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar { visible: false }
                boundsBehavior: Flickable.StopAtBounds
            }
        }
    }

    Connections {
        target: bridge

        function onLogChanged() {
            logModel.clear();
            let currentLog = bridge.terminalLog;
            for (let i = 0; i < currentLog.length; i++) {
                logModel.append({"line": currentLog[i]});
            }
        }

        function onIsWaitingChanged() {
            if (bridge.isWaiting) {
                inputField.opacity = 0.5;
            } else {
                inputField.opacity = 1.0;
                inputField.focus = true;
            }
        }
    }

    function processCommand(userInput) {
        bridge.sendCommand(userInput);
    }
}
