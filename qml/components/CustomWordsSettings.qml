pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../styles"

/**
 * Custom Words settings — words that speech recognition often gets wrong.
 * Similar-sounding transcription errors are auto-corrected.
 */
Column {
    id: root
    spacing: Theme.spacingMedium
    width: parent ? parent.width : 400

    property var wordsList: textPostProcessor.customWords()

    Connections {
        target: textPostProcessor
        function onCustomWordsChanged() {
            root.wordsList = textPostProcessor.customWords()
        }
    }

    // ── Section Header ──
    Text {
        text: qsTr("Custom Words")
        font.pixelSize: Typography.fontSizeLarge
        font.weight: Font.DemiBold
        color: Colors.textPrimary
    }

    Text {
        text: qsTr("Words that your speech recognition often gets wrong. Similar-sounding errors are automatically corrected.")
        font.pixelSize: Typography.fontSizeSmall
        color: Colors.textSecondary
        wrapMode: Text.WordWrap
        width: parent.width
    }

    // ── Word List ──
    Flow {
        width: parent.width
        spacing: Theme.spacingSmall

        Repeater {
            model: root.wordsList

            Rectangle {
                id: wordBadge
                required property int index
                required property var modelData

                width: wordRow.implicitWidth + Theme.spacingMedium * 2
                height: Theme.buttonHeightSmall
                radius: Theme.radiusLarge
                color: Colors.withAlpha(Colors.surface, 0.6)
                border.width: Theme.borderThin
                border.color: Colors.border

                RowLayout {
                    id: wordRow
                    anchors.centerIn: parent
                    spacing: Theme.spacingXS

                    Text {
                        text: wordBadge.modelData
                        font.pixelSize: Typography.fontSizeSmall
                        color: Colors.textPrimary
                    }

                    Text {
                        text: "\u00D7" // × symbol
                        font.pixelSize: Typography.fontSizeMedium
                        color: removeArea.containsMouse ? Colors.error : Colors.textSecondary
                        opacity: removeArea.containsMouse ? 1.0 : 0.6

                        Behavior on color { ColorAnimation { duration: Theme.durationFast } }
                        Behavior on opacity { NumberAnimation { duration: Theme.durationFast } }

                        MouseArea {
                            id: removeArea
                            anchors.fill: parent
                            anchors.margins: -4
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: textPostProcessor.removeCustomWord(wordBadge.index)
                        }
                    }
                }
            }
        }
    }

    // ── Add Word Row ──
    Rectangle {
        width: parent.width
        height: addRow.implicitHeight + Theme.spacingMedium * 2
        radius: Theme.radiusMedium
        color: Colors.withAlpha(Colors.surface, 0.4)
        border.width: Theme.borderThin
        border.color: Colors.border

        RowLayout {
            id: addRow
            anchors.fill: parent
            anchors.margins: Theme.spacingMedium
            spacing: Theme.spacingSmall

            TextField {
                id: newWordField
                Layout.fillWidth: true
                placeholderText: qsTr("New word...")
                font.pixelSize: Typography.fontSizeSmall
                color: Colors.textPrimary
                placeholderTextColor: Colors.textTertiary
                background: Rectangle {
                    radius: Theme.radiusSmall
                    color: Colors.withAlpha(Colors.surface, 0.6)
                    border.width: Theme.borderThin
                    border.color: newWordField.activeFocus ? Colors.accent : Colors.border
                }

                onAccepted: {
                    if (text.trim().length > 0) {
                        textPostProcessor.addCustomWord(text.trim())
                        text = ""
                    }
                }
            }

            Rectangle {
                width: Math.max(80, addBtnText.implicitWidth + Theme.spacingMedium * 2)
                height: Theme.buttonHeightSmall
                radius: Theme.radiusSmall
                color: addBtnArea.pressed
                       ? Colors.withAlpha(Colors.accent, 0.3)
                       : addBtnArea.containsMouse
                         ? Colors.withAlpha(Colors.accent, 0.2)
                         : Colors.withAlpha(Colors.accent, 0.1)

                Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                Text {
                    id: addBtnText
                    anchors.centerIn: parent
                    text: qsTr("Add")
                    font.pixelSize: Typography.fontSizeSmall
                    font.weight: Font.Medium
                    color: Colors.accent
                }

                MouseArea {
                    id: addBtnArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (newWordField.text.trim().length > 0) {
                            textPostProcessor.addCustomWord(newWordField.text.trim())
                            newWordField.text = ""
                        }
                    }
                }
            }
        }
    }

    // ── Info Text ──
    Text {
        text: qsTr("Minimum 3 characters per word. Duplicates are ignored.")
        font.pixelSize: Typography.fontSizeXSmall
        color: Colors.textTertiary
        width: parent.width
    }
}
