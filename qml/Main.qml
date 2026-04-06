pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Window
import "styles"
import "components"

ApplicationWindow {
    id: mainWindow
    width: 720
    height: 640
    minimumWidth: 520
    minimumHeight: 480
    visible: true
    title: "TalkScribe"
    color: Colors.background

    // ── Overlay (floating dictation panel) ──
    DictationOverlay {
        id: dictationOverlay
    }

    // ── Tab bar ──
    header: Rectangle {
        height: 48
        color: Colors.surface

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.spacingMedium
            anchors.rightMargin: Theme.spacingMedium
            spacing: 4

            Repeater {
                model: [
                    { label: qsTr("Dictation"), icon: "\uD83C\uDFA4" },
                    { label: qsTr("Models"), icon: "\uD83E\uDDE0" },
                    { label: qsTr("Custom Words"), icon: "\uD83D\uDCD6" },
                    { label: qsTr("Snippets"), icon: "\u2702" },
                    { label: qsTr("Statistics"), icon: "\uD83D\uDCCA" }
                ]

                Rectangle {
                    required property var modelData
                    required property int index
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: tabStack.currentIndex === index
                        ? Colors.withAlpha(Colors.accent, 0.12)
                        : tabMa.containsMouse
                          ? Colors.withAlpha(Colors.surface, 0.8)
                          : "transparent"
                    radius: Theme.radiusSmall

                    Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 2

                        Text {
                            text: modelData.icon
                            font.pixelSize: 16
                            Layout.alignment: Qt.AlignHCenter
                        }
                        Text {
                            text: modelData.label
                            font.pixelSize: Typography.fontSizeXSmall
                            font.weight: tabStack.currentIndex === index ? Font.DemiBold : Font.Normal
                            color: tabStack.currentIndex === index
                                ? Colors.accent
                                : Colors.textSecondary
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }

                    MouseArea {
                        id: tabMa
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: tabStack.currentIndex = index
                    }
                }
            }
        }

        // Bottom border
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: Colors.border
        }
    }

    // ── Content ──
    StackLayout {
        id: tabStack
        anchors.fill: parent

        // Tab 0: Dictation Settings
        Flickable {
            contentHeight: dictSettingsCol.implicitHeight + Theme.spacingLarge * 2
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ColumnLayout {
                id: dictSettingsCol
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: Theme.spacingLarge
                spacing: 0

                DictationSettings {}
            }
        }

        // Tab 1: Models
        Flickable {
            contentHeight: modelCol.implicitHeight + Theme.spacingLarge * 2
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ColumnLayout {
                id: modelCol
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: Theme.spacingLarge
                spacing: 0

                TranscriptionModelSettings {}
            }
        }

        // Tab 2: Custom Words
        Flickable {
            contentHeight: wordsCol.implicitHeight + Theme.spacingLarge * 2
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ColumnLayout {
                id: wordsCol
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: Theme.spacingLarge
                spacing: 0

                CustomWordsSettings {}
            }
        }

        // Tab 3: Snippets
        Flickable {
            contentHeight: snippetCol.implicitHeight + Theme.spacingLarge * 2
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ColumnLayout {
                id: snippetCol
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: Theme.spacingLarge
                spacing: 0

                SnippetSettings {}
            }
        }

        // Tab 4: Statistics
        Flickable {
            contentHeight: statsCol.implicitHeight + Theme.spacingLarge * 2
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ColumnLayout {
                id: statsCol
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: Theme.spacingLarge
                spacing: 0

                DictationStatsSection {}
            }
        }
    }
}
