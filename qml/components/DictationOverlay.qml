pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt.labs.settings
import "../styles"

/**
 * Compact floating dictation overlay — always on top, draggable.
 *
 * States:
 *   idle         → small mic button, waiting for hotkey/click
 *   loading      → spinner while model loads from disk
 *   recording    → X + waveform bars + stop button
 *   transcribing → spinner + "Transkribiere..." (min 1s visible)
 *   done         → brief "Eingefuegt!" flash, then back to idle
 */
Item {
    id: root

    function closeOverlay() {
        dictationWin.visible = false;
    }

    // ── Saved position ──
    Settings {
        id: overlaySettings
        category: "DictationOverlay"
        property real savedX: -1
        property real savedY: -1
    }

    // ── State machine ──
    property string _state: "idle" // idle | loading | recording | transcribing | done
    property int _winWidth: 56 // driven by state, bound to dictationWin.width
    property bool _pasteSucceeded: false
    property bool _emptyResult: false
    property int _prevWinWidth: 56

    // Keep overlay centered when width changes (expand symmetrically left+right)
    on_WinWidthChanged: {
        if (dictationWin._targetX >= 0 && _prevWinWidth !== _winWidth) {
            var delta = _winWidth - _prevWinWidth
            var newX = dictationWin._targetX - Math.round(delta / 2)
            // Clamp to screen bounds (Screen is attached to Window, not Item)
            var screenW = dictationWin.screen ? dictationWin.screen.desktopAvailableWidth : 1920
            newX = Math.max(0, Math.min(newX, screenW - _winWidth))
            dictationWin._targetX = newX
            dictationWin.x = newX
        }
        _prevWinWidth = _winWidth
    }

    // Recording timer
    property int _recordingSeconds: 0
    Timer {
        id: recordingTimer
        interval: 1000
        repeat: true
        running: root._state === "recording"
        onTriggered: root._recordingSeconds++
    }

    // Auto-hide "done" state after 1.5s
    Timer {
        id: doneTimer
        interval: 1500
        onTriggered: {
            root._state = "idle"
            root._winWidth = 56
        }
    }

    // ── Pending completion: show "transcribing" for at least 1s ──
    property string _pendingFinalText: ""
    Timer {
        id: transcribingMinTimer
        interval: 1000
        onTriggered: {
            if (root._pendingFinalText.length > 0) {
                root._emptyResult = (root._pendingFinalText === "__empty__")
                root._state = "done"
                root._winWidth = 180
                doneTimer.start()
                root._pendingFinalText = ""
            }
            // If no pending text yet, stay in "transcribing" — dictationComplete
            // will transition to "done" or "idle" when it arrives.
        }
    }

    // ── Audio level history for waveform bars ──
    property var _levelHistory: []
    readonly property int _barCount: 20

    Timer {
        id: levelSampler
        interval: 80
        repeat: true
        running: root._state === "recording"
        onTriggered: {
            var hist = root._levelHistory.slice()
            hist.push(globalDictationService.audioLevel)
            if (hist.length > root._barCount)
                hist.shift()
            root._levelHistory = hist
        }
    }

    Connections {
        target: globalDictationService
        function onIsActiveChanged() {
            if (globalDictationService.isActive) {
                root._state = "recording"
                root._recordingSeconds = 0
                root._levelHistory = new Array(root._barCount).fill(0.0)
                root._winWidth = 320
                // Don't set visible imperatively — binding handles it.
                // State is now "recording" so binding evaluates to true.
                macWindowHelper.raiseFloatingPanel(dictationWin)
            } else if (root._state === "recording") {
                // Stop pressed — show "transcribing" for at least 1s
                root._state = "transcribing"
                root._winWidth = 220
                transcribingMinTimer.start()
                transcribingTimeout.start()
            }
        }
        function onIsStoppingChanged() {
            if (globalDictationService.isStopping && root._state === "recording") {
                // Immediately show "transcribing" — don't wait for background thread
                root._state = "transcribing"
                root._winWidth = 220
                transcribingMinTimer.start()
                transcribingTimeout.start()
            }
        }
        function onIsLoadingChanged() {
            if (globalDictationService.isLoading) {
                root._state = "loading"
                root._winWidth = 180
                // Don't set visible imperatively — binding handles it.
                macWindowHelper.raiseFloatingPanel(dictationWin)
            }
        }
        function onDictationComplete() {
            transcribingTimeout.stop()
            // Ignore late signals from cancelled recordings
            if (root._state === "idle" || root._state === "loading") return

            var text = globalDictationService?.finalText ?? ""
            root._pasteSucceeded = true // assume success, onErrorOccurred sets false
            root._emptyResult = (text.length === 0)

            if (root._emptyResult) {
                // No speech — show feedback immediately, don't wait for min timer
                transcribingMinTimer.stop()
                root._state = "done"
                root._winWidth = 180
                doneTimer.start()
            } else if (root._state === "transcribing" && transcribingMinTimer.running) {
                // Min timer still running — queue, timer will show "done"
                root._pendingFinalText = text
            } else {
                // Show done (with text)
                root._state = "done"
                root._winWidth = 180
                doneTimer.start()
            }
        }
        function onErrorOccurred() {
            transcribingMinTimer.stop()
            transcribingTimeout.stop()
            // If text was copied to clipboard, still show "done" confirmation
            var text = globalDictationService?.finalText ?? ""
            if (text.length > 0) {
                root._pasteSucceeded = false
                root._state = "done"
                root._winWidth = 180
                doneTimer.start()
            } else {
                root._state = "idle"
                root._winWidth = 56
            }
        }
        function onTextPasted() {
            root._pasteSucceeded = true
        }
        function onOverlayPositionResetRequested() {
            overlaySettings.savedX = -1
            overlaySettings.savedY = -1
            if (dictationWin) {
                dictationWin._targetX = dictationWin._defaultX()
                dictationWin._targetY = dictationWin._defaultY()
                dictationWin._applyPosition()
            }
        }
    }

    // ── Transcribing fallback timeout — prevents stuck state ──
    Timer {
        id: transcribingTimeout
        interval: 15000
        onTriggered: {
            if (root._state === "transcribing") {
                root._emptyResult = true
                root._state = "done"
                root._winWidth = 180
                doneTimer.start()
            }
        }
    }

    // ── Floating Window ──
    Window {
        id: dictationWin
        visible: (globalDictationService?.enabled ?? false) && ((globalDictationService?.overlayVisible ?? true) || root._state !== "idle")
        color: "transparent"
        transientParent: null
        flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus

        // Fixed size per state — no user resize
        width: root._winWidth
        height: 56
        minimumWidth: root._winWidth
        maximumWidth: root._winWidth
        minimumHeight: 56
        maximumHeight: 56

        // Track desired position separately — macOS may move the NSWindow
        // (e.g. after configureFloatingPanel), so we re-apply after each such call.
        property real _targetX: -1
        property real _targetY: -1

        function _applyPosition() {
            if (_targetX >= 0 && _targetY >= 0) {
                x = _targetX
                y = _targetY
            }
        }

        // Get the screen of the main window (not the virtual desktop spanning all monitors)
        function _mainScreenWidth() {
            var s = mainWindow?.screen
            return s ? s.desktopAvailableWidth : Screen.desktopAvailableWidth
        }
        function _mainScreenHeight() {
            var s = mainWindow?.screen
            return s ? s.desktopAvailableHeight : Screen.desktopAvailableHeight
        }
        // Default position: bottom-right of the main window's screen
        function _defaultX() { return _mainScreenWidth() - 80 }
        function _defaultY() { return _mainScreenHeight() - 80 }

        Component.onCompleted: {
            // Restore saved position or use default on main window's screen
            if (overlaySettings.savedX >= 0 && overlaySettings.savedY >= 0) {
                var maxX = _mainScreenWidth() - width
                var maxY = _mainScreenHeight() - height
                if (overlaySettings.savedX <= maxX && overlaySettings.savedY <= maxY) {
                    _targetX = overlaySettings.savedX
                    _targetY = overlaySettings.savedY
                } else {
                    overlaySettings.savedX = -1
                    overlaySettings.savedY = -1
                    _targetX = _defaultX()
                    _targetY = _defaultY()
                }
            } else {
                _targetX = _defaultX()
                _targetY = _defaultY()
            }
            _applyPosition()
            macWindowHelper.configureFloatingPanel(dictationWin)
            // Re-apply after configure — macOS may have moved it
            _applyPosition()
            console.log("[OVERLAY] onCompleted — pos:", x, y, "target:", _targetX, _targetY)
        }
        onVisibleChanged: {
            console.log("[OVERLAY] visible changed to:", visible, "pos:", x, y, "target:", _targetX, _targetY)
            if (visible) {
                macWindowHelper.configureFloatingPanel(dictationWin)
                _applyPosition()
                macWindowHelper.raiseFloatingPanel(dictationWin)
                _applyPosition()
                // Delayed re-apply — macOS sometimes repositions async
                posFixTimer.start()
            }
        }

        Timer {
            id: posFixTimer
            interval: 50
            repeat: false
            onTriggered: dictationWin._applyPosition()
        }

        // ── Main content ──
        Rectangle {
            id: pill
            anchors.fill: parent
            radius: height / 2
            color: root._state === "done" ? Colors.success : Colors.surfaceDark
            border.width: Theme.borderThin
            border.color: Colors.withAlpha(Colors.border, 0.5)

            Behavior on color { ColorAnimation { duration: Theme.durationFast } }

            // Drag to move — uses screen-space coordinates to avoid oscillation
            MouseArea {
                id: dragArea
                anchors.fill: parent
                preventStealing: true
                hoverEnabled: true
                cursorShape: _dragged ? Qt.ClosedHandCursor
                                     : (containsMouse ? Qt.OpenHandCursor : Qt.ArrowCursor)
                property real _startScreenX: 0
                property real _startScreenY: 0
                property real _startWinX: 0
                property real _startWinY: 0
                property bool _dragged: false

                onPressed: function(mouse) {
                    // Convert to screen coordinates — immune to window movement
                    var global = mapToGlobal(mouse.x, mouse.y)
                    _startScreenX = global.x
                    _startScreenY = global.y
                    _startWinX = dictationWin.x
                    _startWinY = dictationWin.y
                    _dragged = false
                    console.log("[OVERLAY] Drag pressed — win:", _startWinX, _startWinY,
                                "screen:", _startScreenX, _startScreenY)
                }
                onPositionChanged: function(mouse) {
                    if (!pressed) return  // hover only — don't move
                    var global = mapToGlobal(mouse.x, mouse.y)
                    var dx = global.x - _startScreenX
                    var dy = global.y - _startScreenY
                    if (Math.abs(dx) > 3 || Math.abs(dy) > 3)
                        _dragged = true
                    if (_dragged) {
                        var newX = Math.round(_startWinX + dx)
                        var newY = Math.round(_startWinY + dy)
                        dictationWin.x = newX
                        dictationWin.y = newY
                        dictationWin._targetX = newX
                        dictationWin._targetY = newY
                    }
                }
                onReleased: {
                    console.log("[OVERLAY] Drag released — dragged:", _dragged,
                                "finalPos:", dictationWin.x, dictationWin.y)
                    if (_dragged) {
                        overlaySettings.savedX = dictationWin.x
                        overlaySettings.savedY = dictationWin.y
                    }
                }
                onClicked: {
                    if (_dragged) return
                    if (root._state === "idle") {
                        globalDictationService.toggle()
                    } else if (root._state === "done") {
                        doneTimer.stop()
                        root._state = "idle"
                        root._winWidth = 56
                        globalDictationService.toggle()
                    }
                }
            }

            // ── Drag handle grip bar — visible on hover in idle ──
            Rectangle {
                opacity: (root._state === "idle" && dragArea.containsMouse) ? 0.5 : 0.0
                width: 16; height: 3; radius: 1.5
                color: Colors.textSecondary
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                Behavior on opacity { NumberAnimation { duration: 150 } }
            }

            RowLayout {
                anchors.centerIn: parent
                spacing: Theme.spacingSmall

                // ═══════════════════════════════════
                // ── IDLE: Mic icon ──
                // ═══════════════════════════════════
                Image {
                    visible: root._state === "idle"
                    source: "qrc:/qt/qml/TalkLess/resources/icons/microphone.svg"
                    sourceSize: Qt.size(24, 24)
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    Layout.alignment: Qt.AlignVCenter
                }

                // ═══════════════════════════════════
                // ── LOADING: spinner + text ──
                // ═══════════════════════════════════
                BusyIndicator {
                    visible: root._state === "loading"
                    running: root._state === "loading"
                    Layout.preferredWidth: 20
                    Layout.preferredHeight: 20
                    Layout.alignment: Qt.AlignVCenter
                    palette.dark: Colors.accent
                }
                Text {
                    visible: root._state === "loading"
                    text: qsTr("Loading...")
                    font.pixelSize: Typography.fontSizeSmall
                    color: Colors.textSecondary
                    Layout.alignment: Qt.AlignVCenter
                }

                // ═══════════════════════════════════
                // ── RECORDING: cancel + waveform + duration + stop ──
                // ═══════════════════════════════════

                // Cancel button (X) — left
                Rectangle {
                    visible: root._state === "recording"
                    width: 36; height: 36; radius: 18
                    color: cancelArea.containsMouse
                           ? Colors.withAlpha(Colors.textSecondary, 0.3)
                           : Colors.withAlpha(Colors.textSecondary, 0.15)
                    Layout.alignment: Qt.AlignVCenter

                    Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                    Text {
                        anchors.centerIn: parent
                        text: "\u2715"
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        color: Colors.textPrimary
                    }
                    MouseArea {
                        id: cancelArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            globalDictationService.stop()
                            globalDictationService.clearText()
                            transcribingMinTimer.stop()
                            root._state = "idle"
                            root._winWidth = 56
                        }
                    }
                }

                // Waveform bars — center
                Row {
                    visible: root._state === "recording"
                    spacing: 2
                    Layout.alignment: Qt.AlignVCenter
                    Layout.fillWidth: true

                    Repeater {
                        model: root._barCount
                        Rectangle {
                            required property int index
                            width: 3
                            height: {
                                var lvl = index < root._levelHistory.length
                                    ? root._levelHistory[index] : 0.0
                                return Math.max(4, 28 * lvl)
                            }
                            radius: 1.5
                            color: Colors.textPrimary
                            anchors.verticalCenter: parent ? parent.verticalCenter : undefined
                            Behavior on height { NumberAnimation { duration: 60; easing.type: Easing.OutQuad } }
                        }
                    }
                }

                // Duration
                Text {
                    visible: root._state === "recording"
                    text: {
                        var m = Math.floor(root._recordingSeconds / 60)
                        var s = root._recordingSeconds % 60
                        return (m < 10 ? "0" : "") + m + ":" + (s < 10 ? "0" : "") + s
                    }
                    font.pixelSize: Typography.fontSizeSmall
                    font.weight: Font.DemiBold
                    font.family: "monospace"
                    color: Colors.textSecondary
                    Layout.alignment: Qt.AlignVCenter
                }

                // Stop button — right (big red circle with white square)
                Rectangle {
                    visible: root._state === "recording"
                    width: 44; height: 44; radius: 22
                    color: stopArea.containsMouse
                           ? Qt.lighter(Colors.error, 1.15)
                           : Colors.error
                    Layout.alignment: Qt.AlignVCenter

                    Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                    Rectangle {
                        anchors.centerIn: parent
                        width: 14; height: 14; radius: 3
                        color: "white"
                    }

                    MouseArea {
                        id: stopArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            // Immediately show transcribing state — don't wait
                            // for isActiveChanged (delayed 250ms to capture last audio)
                            root._state = "transcribing"
                            root._winWidth = 220
                            transcribingMinTimer.start()
                            globalDictationService.stop()
                        }
                    }
                }

                // ═══════════════════════════════════
                // ── TRANSCRIBING: spinner + text + cancel ──
                // ═══════════════════════════════════
                BusyIndicator {
                    visible: root._state === "transcribing"
                    running: root._state === "transcribing"
                    Layout.preferredWidth: 20
                    Layout.preferredHeight: 20
                    Layout.alignment: Qt.AlignVCenter
                    palette.dark: Colors.accent
                }
                Text {
                    visible: root._state === "transcribing"
                    text: qsTr("Transcribing...")
                    font.pixelSize: Typography.fontSizeSmall
                    color: Colors.textPrimary
                    Layout.alignment: Qt.AlignVCenter
                }
                Text {
                    visible: root._state === "transcribing"
                    text: "\u2715"
                    font.pixelSize: Typography.fontSizeSmall
                    color: Colors.textSecondary
                    Layout.alignment: Qt.AlignVCenter

                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: -4
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            globalDictationService.clearText()
                            transcribingMinTimer.stop()
                            root._state = "idle"
                            root._winWidth = 56
                        }
                    }
                }

                // ═══════════════════════════════════
                // ── DONE: checkmark ──
                // ═══════════════════════════════════
                Text {
                    visible: root._state === "done"
                    text: root._emptyResult
                          ? "\u2715 " + qsTr("No speech")
                          : "\u2713 " + (root._pasteSucceeded ? qsTr("Pasted!") : qsTr("Copied!"))
                    font.pixelSize: Typography.fontSizeSmall
                    font.weight: Font.DemiBold
                    color: "white"
                    Layout.alignment: Qt.AlignVCenter
                }
            }
        }
    }
}
