pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../styles"

/**
 * Anywhere Dictation settings — hotkey, auto-paste, overlay config.
 */
Column {
    id: root
    spacing: Theme.spacingMedium
    width: parent ? parent.width : 400

    // TalkScribe: no HotkeyManager — overlay click to toggle

    // overlayVisible is now on globalDictationService (C++ single source of truth)

    // ── Section Header ──
    Text {
        text: qsTr("Anywhere Dictation")
        font.pixelSize: Typography.fontSizeLarge
        font.weight: Font.DemiBold
        color: Colors.textPrimary
    }

    Text {
        text: qsTr("Press a hotkey anywhere to start dictating. Your speech is transcribed locally and copied to the clipboard.")
        font.pixelSize: Typography.fontSizeSmall
        color: Colors.textSecondary
        wrapMode: Text.WordWrap
        width: parent.width
    }

    // ── Usage Info ──
    Rectangle {
        width: parent.width
        height: usageRow.implicitHeight + Theme.spacingMedium * 2
        radius: Theme.radiusMedium
        color: Colors.withAlpha(Colors.surface, 0.4)
        border.width: Theme.borderThin
        border.color: Colors.border

        RowLayout {
            id: usageRow
            anchors.fill: parent
            anchors.margins: Theme.spacingMedium
            spacing: Theme.spacingMedium

            Column {
                Layout.fillWidth: true
                spacing: Theme.spacingXXS

                Text {
                    text: qsTr("How to use")
                    font.pixelSize: Typography.fontSizeMedium
                    font.weight: Font.Medium
                    color: Colors.textPrimary
                }
                Text {
                    text: qsTr("Click the microphone in the overlay to start dictating. Click again to stop. Text is automatically copied to the clipboard.")
                    font.pixelSize: Typography.fontSizeSmall
                    color: Colors.textSecondary
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
            }
        }
    }

    // ── Texteingabe Section ──
    Text {
        text: qsTr("Text Input")
        font.pixelSize: Typography.fontSizeMedium
        font.weight: Font.DemiBold
        color: Colors.textPrimary
        topPadding: Theme.spacingSmall
    }

    Rectangle {
        width: parent.width
        height: pasteCol.implicitHeight + Theme.spacingMedium * 2
        radius: Theme.radiusMedium
        color: Colors.withAlpha(Colors.surface, 0.4)
        border.width: Theme.borderThin
        border.color: Colors.border

        Column {
            id: pasteCol
            anchors.fill: parent
            anchors.margins: Theme.spacingMedium
            spacing: Theme.spacingSmall

            // Auto-paste toggle
            RowLayout {
                width: parent.width
                spacing: Theme.spacingSmall

                Column {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Auto-paste text")
                        font.pixelSize: Typography.fontSizeMedium
                        color: Colors.textPrimary
                    }
                    Text {
                        text: qsTr("When disabled, text is only copied to the clipboard.")
                        font.pixelSize: Typography.fontSizeSmall
                        color: Colors.textSecondary
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }
                }
                Switch {
                    checked: globalDictationService?.autoPaste ?? true
                    onToggled: globalDictationService.setAutoPaste(checked)
                }
            }
        }
    }

    // ── Overlay Section ──
    Text {
        text: qsTr("Overlay")
        font.pixelSize: Typography.fontSizeMedium
        font.weight: Font.DemiBold
        color: Colors.textPrimary
        topPadding: Theme.spacingSmall
    }

    Rectangle {
        width: parent.width
        height: overlayCol.implicitHeight + Theme.spacingMedium * 2
        radius: Theme.radiusMedium
        color: Colors.withAlpha(Colors.surface, 0.4)
        border.width: Theme.borderThin
        border.color: Colors.border

        Column {
            id: overlayCol
            anchors.fill: parent
            anchors.margins: Theme.spacingMedium
            spacing: Theme.spacingSmall

            // Show overlay toggle
            RowLayout {
                width: parent.width
                spacing: Theme.spacingSmall

                Column {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Show overlay")
                        font.pixelSize: Typography.fontSizeMedium
                        color: Colors.textPrimary
                    }
                    Text {
                        text: qsTr("Recording is still available via hotkey when disabled.")
                        font.pixelSize: Typography.fontSizeSmall
                        color: Colors.textSecondary
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }
                }
                Switch {
                    checked: globalDictationService?.overlayVisible ?? true
                    onToggled: {
                        if (globalDictationService)
                            globalDictationService.overlayVisible = checked
                    }
                }
            }

            // Separator
            Rectangle { width: parent.width; height: 1; color: Colors.withAlpha(Colors.border, 0.3) }

            // Reset position
            RowLayout {
                width: parent.width
                spacing: Theme.spacingSmall

                Column {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Reset position")
                        font.pixelSize: Typography.fontSizeMedium
                        color: Colors.textPrimary
                    }
                    Text {
                        text: qsTr("Move the overlay back to the default position.")
                        font.pixelSize: Typography.fontSizeSmall
                        color: Colors.textSecondary
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }
                }

                Rectangle {
                    width: resetLabel.implicitWidth + Theme.spacingMedium * 2
                    height: Theme.buttonHeightSmall
                    radius: Theme.radiusSmall
                    color: resetArea.pressed
                           ? Colors.withAlpha(Colors.surface, 0.9)
                           : resetArea.containsMouse
                             ? Colors.withAlpha(Colors.surface, 0.7)
                             : Colors.withAlpha(Colors.surface, 0.5)
                    border.width: Theme.borderThin
                    border.color: Colors.border
                    Layout.alignment: Qt.AlignVCenter

                    Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                    Text {
                        id: resetLabel
                        anchors.centerIn: parent
                        text: qsTr("Reset")
                        font.pixelSize: Typography.fontSizeSmall
                        color: Colors.textPrimary
                    }

                    MouseArea {
                        id: resetArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            globalDictationService.requestOverlayPositionReset()
                        }
                    }
                }
            }
        }
    }

    // ── Model requirement note ──
    Rectangle {
        visible: {
            var models = whisperModelManager?.availableModels ?? []
            for (var i = 0; i < models.length; i++) {
                if (models[i].downloaded) return false
            }
            return true
        }
        width: parent.width
        height: noteCol.implicitHeight + Theme.spacingSmall * 2
        radius: Theme.radiusSmall
        color: Colors.withAlpha(Colors.warning, 0.08)
        border.width: Theme.borderThin
        border.color: Colors.withAlpha(Colors.warning, 0.3)

        Column {
            id: noteCol
            anchors.fill: parent
            anchors.margins: Theme.spacingSmall
            spacing: Theme.spacingXXS

            Text {
                text: qsTr("Model required")
                font.pixelSize: Typography.fontSizeSmall
                font.weight: Font.DemiBold
                color: Colors.warning
            }
            Text {
                text: qsTr("Download a speech model above to use Dictation.")
                font.pixelSize: Typography.fontSizeSmall
                color: Colors.textSecondary
            }
        }
    }
}
