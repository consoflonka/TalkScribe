pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../styles"

/**
 * Global transcription model settings — always visible, not gated by any toggle.
 * Models, download, GPU status, threads, language detection.
 * Extracted from WhisperModelSettings.qml to make models accessible
 * regardless of whether Live Analysis is enabled.
 */
Column {
    id: root
    spacing: Theme.spacingMedium
    width: parent ? parent.width : 400

    // Delete confirmation state
    property string deleteConfirmModelId: ""
    property string deleteConfirmModelName: ""

    // ── Delete Confirmation Dialog ──
    Dialog {
        id: deleteConfirmDialog
        anchors.centerIn: Overlay.overlay
        title: qsTr("Delete model?")
        modal: true
        standardButtons: Dialog.Yes | Dialog.No

        contentItem: Text {
            text: qsTr("Delete \"%1\" and free up disk space?\n\nYou can re-download it anytime.").arg(root.deleteConfirmModelName)
            font.pixelSize: Typography.fontSizeSmall
            color: Colors.textPrimary
            wrapMode: Text.WordWrap
            width: 280
        }

        onAccepted: {
            if (root.deleteConfirmModelId.length > 0)
                whisperModelManager.deleteModel(root.deleteConfirmModelId)
            root.deleteConfirmModelId = ""
        }
        onRejected: root.deleteConfirmModelId = ""
    }

    // ── Section Header ──
    Text {
        text: qsTr("Speech Models")
        font.pixelSize: Typography.fontSizeLarge
        font.weight: Font.DemiBold
        color: Colors.textPrimary
    }

    Text {
        text: qsTr("Download models for local speech recognition. Used by Dictation, Live Analysis, and offline transcription.")
        font.pixelSize: Typography.fontSizeSmall
        color: Colors.textSecondary
        wrapMode: Text.WordWrap
        width: parent.width
    }

    // ── Model Selection ──
    Column {
        width: parent.width
        spacing: Theme.spacingSmall

        // ── Empty State: No model downloaded ──
        Rectangle {
            visible: {
                var models = whisperModelManager?.availableModels ?? []
                for (var i = 0; i < models.length; i++) {
                    if (models[i].downloaded) return false
                }
                return true
            }
            width: parent.width
            height: emptyCol.implicitHeight + Theme.spacingMedium * 2
            radius: Theme.radiusMedium
            color: Colors.withAlpha(Colors.info, 0.08)
            border.width: Theme.borderThin
            border.color: Colors.withAlpha(Colors.info, 0.3)

            Column {
                id: emptyCol
                anchors.centerIn: parent
                spacing: Theme.spacingSmall
                width: parent.width - Theme.spacingMedium * 2

                Text {
                    text: qsTr("No model downloaded yet")
                    font.pixelSize: Typography.fontSizeMedium
                    font.weight: Font.DemiBold
                    color: Colors.textPrimary
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    text: qsTr("Download a model to enable local speech recognition. We recommend starting with Whisper Large V3 Turbo (809 MB).")
                    font.pixelSize: Typography.fontSizeSmall
                    color: Colors.textSecondary
                    wrapMode: Text.WordWrap
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
                Rectangle {
                    width: Math.max(180, dlRecLabel.implicitWidth + Theme.spacingMedium * 2)
                    height: Theme.buttonHeightMedium
                    radius: Theme.radiusSmall
                    color: dlRecArea.pressed ? Qt.darker(Colors.accent, 1.1)
                           : dlRecArea.containsMouse ? Colors.accent : Colors.withAlpha(Colors.accent, 0.9)
                    anchors.horizontalCenter: parent.horizontalCenter

                    Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                    Text {
                        id: dlRecLabel
                        anchors.centerIn: parent
                        text: qsTr("Download recommended model")
                        font.pixelSize: Typography.fontSizeSmall
                        font.weight: Font.DemiBold
                        color: Colors.textOnAccent
                    }
                    MouseArea {
                        id: dlRecArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: whisperModelManager.downloadModel("large-v3-turbo")
                    }
                }
            }
        }

        // ── Download Error Banner ──
        Rectangle {
            visible: (whisperModelManager?.downloadError ?? "").length > 0
            width: parent.width
            height: errorCol.implicitHeight + Theme.spacingSmall * 2
            radius: Theme.radiusSmall
            color: Colors.withAlpha(Colors.error, 0.1)
            border.width: Theme.borderThin
            border.color: Colors.withAlpha(Colors.error, 0.3)

            Column {
                id: errorCol
                anchors.fill: parent
                anchors.margins: Theme.spacingSmall
                spacing: 4

                Text {
                    text: qsTr("Download failed")
                    font.pixelSize: Typography.fontSizeSmall
                    font.weight: Font.DemiBold
                    color: Colors.error
                }
                Text {
                    text: whisperModelManager?.downloadError ?? ""
                    font.pixelSize: Typography.fontSizeSmall
                    color: Colors.textSecondary
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
                Text {
                    text: qsTr("Try again")
                    font.pixelSize: Typography.fontSizeSmall
                    color: Colors.accent
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: whisperModelManager.downloadModel(whisperModelManager.activeModelId)
                    }
                }
            }
        }

        Repeater {
            model: whisperModelManager?.availableModels ?? []

            Rectangle {
                required property var modelData
                required property int index
                width: parent.width
                height: modelContent.implicitHeight + Theme.spacingMedium * 2
                radius: Theme.radiusMedium
                color: isActive
                       ? Colors.withAlpha(Colors.accent, 0.08)
                       : Colors.withAlpha(Colors.surface, 0.4)
                border.width: isActive ? Theme.borderMedium : Theme.borderThin
                border.color: isActive
                              ? Colors.accent
                              : Colors.border

                property bool isDownloaded: modelData.downloaded ?? false
                property bool isActive: isDownloaded && (whisperModelManager?.activeModelId ?? "") === modelData.id
                property bool isLocked: modelData.locked ?? false

                Behavior on border.color { ColorAnimation { duration: Theme.durationFast } }
                Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                Column {
                    id: modelContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: Theme.spacingMedium
                    spacing: Theme.spacingSmall

                    // ── Row 1: Radio + Name + Badge + Actions ──
                    RowLayout {
                        width: parent.width
                        spacing: Theme.spacingSmall

                        Rectangle {
                            width: 20; height: 20; radius: 10
                            color: "transparent"
                            border.width: Theme.borderMedium
                            border.color: isActive ? Colors.accent : Colors.textSecondary
                            Layout.alignment: Qt.AlignVCenter

                            Rectangle {
                                anchors.centerIn: parent
                                width: 10; height: 10; radius: 5
                                color: Colors.accent
                                visible: isActive
                            }
                        }

                        Text {
                            text: modelData.displayName
                            font.pixelSize: Typography.fontSizeMedium
                            font.weight: Font.DemiBold
                            color: isLocked ? Colors.textSecondary : Colors.textPrimary
                            Layout.alignment: Qt.AlignVCenter
                        }

                        Rectangle {
                            visible: modelData.recommended ?? false
                            width: recLabel.implicitWidth + Theme.spacingSmall * 2
                            height: 22
                            radius: Theme.radiusSmall
                            color: Colors.accent
                            Layout.alignment: Qt.AlignVCenter

                            Text {
                                id: recLabel
                                anchors.centerIn: parent
                                text: qsTr("Recommended")
                                font.pixelSize: Typography.fontSizeXSmall
                                font.weight: Font.DemiBold
                                color: Colors.textOnAccent
                            }
                        }

                        Rectangle {
                            visible: isLocked
                            width: lockLabel.implicitWidth + Theme.spacingSmall * 2
                            height: 22
                            radius: Theme.radiusSmall
                            color: Colors.withAlpha(Colors.warning, 0.15)
                            border.width: Theme.borderThin
                            border.color: Colors.withAlpha(Colors.warning, 0.3)
                            Layout.alignment: Qt.AlignVCenter

                            Text {
                                id: lockLabel
                                anchors.centerIn: parent
                                text: qsTr("Business")
                                font.pixelSize: Typography.fontSizeXSmall
                                color: Colors.warning
                            }
                        }

                        Item { Layout.fillWidth: true }

                        Rectangle {
                            visible: isDownloaded && !isActive
                            width: 28; height: 28
                            radius: Theme.radiusSmall
                            color: delArea.pressed
                                   ? Colors.withAlpha(Colors.error, 0.25)
                                   : delArea.containsMouse
                                     ? Colors.withAlpha(Colors.error, 0.15)
                                     : "transparent"
                            Layout.alignment: Qt.AlignVCenter

                            Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                            Text {
                                anchors.centerIn: parent
                                text: "\u{1F5D1}"
                                font.pixelSize: Typography.fontSizeSmall
                                opacity: delArea.containsMouse ? 1 : 0.5
                            }

                            MouseArea {
                                id: delArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    deleteConfirmModelId = modelData.id
                                    deleteConfirmModelName = modelData.displayName
                                    deleteConfirmDialog.open()
                                }
                            }
                        }

                        // Upgrade button for locked/Business models
                        Rectangle {
                            visible: isLocked && !(modelData.unavailableReason ?? "").length
                            width: Math.max(80, upgradeText.implicitWidth + Theme.spacingMedium * 2)
                            height: Theme.buttonHeightSmall
                            radius: Theme.radiusSmall
                            color: upgradeBtnArea.pressed ? Colors.withAlpha(Colors.warning, 0.3)
                                   : upgradeBtnArea.containsMouse ? Colors.withAlpha(Colors.warning, 0.2)
                                     : Colors.withAlpha(Colors.warning, 0.1)
                            border.width: Theme.borderThin
                            border.color: Colors.withAlpha(Colors.warning, 0.4)
                            Layout.alignment: Qt.AlignVCenter

                            Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                            Text {
                                id: upgradeText
                                anchors.centerIn: parent
                                text: qsTr("Upgrade")
                                font.pixelSize: Typography.fontSizeSmall
                                font.weight: Font.DemiBold
                                color: Colors.warning
                            }
                            MouseArea {
                                id: upgradeBtnArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (typeof licenseClient !== "undefined")
                                        licenseClient.upgradeRequired("whisper_large_models")
                                }
                            }
                        }

                        Rectangle {
                            visible: !(whisperModelManager?.isDownloading ?? false) && !isLocked
                            width: Math.max(80, actionText.implicitWidth + Theme.spacingMedium * 2)
                            height: Theme.buttonHeightSmall
                            radius: Theme.radiusSmall
                            color: !isDownloaded
                                   ? (dlBtnArea.pressed ? Qt.darker(Colors.accent, 1.1) : dlBtnArea.containsMouse ? Colors.accent : Colors.withAlpha(Colors.accent, 0.85))
                                   : isActive
                                     ? Colors.withAlpha(Colors.success, 0.15)
                                     : (dlBtnArea.pressed ? Colors.withAlpha(Colors.surface, 1.0) : dlBtnArea.containsMouse ? Colors.withAlpha(Colors.surface, 0.9) : Colors.withAlpha(Colors.surface, 0.6))
                            border.width: isDownloaded && !isActive ? Theme.borderThin : 0
                            border.color: Colors.border
                            Layout.alignment: Qt.AlignVCenter

                            Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                            Text {
                                id: actionText
                                anchors.centerIn: parent
                                text: !isDownloaded ? qsTr("Download")
                                      : isActive ? qsTr("Active")
                                      : qsTr("Select")
                                font.pixelSize: Typography.fontSizeSmall
                                font.weight: Font.DemiBold
                                color: !isDownloaded ? Colors.textOnAccent
                                       : isActive ? Colors.success
                                       : Colors.textPrimary
                            }

                            MouseArea {
                                id: dlBtnArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: isActive ? Qt.ArrowCursor : Qt.PointingHandCursor
                                enabled: !isActive
                                onClicked: {
                                    console.log("[TranscriptionModelSettings] Button clicked for:", modelData.id,
                                                "downloaded:", isDownloaded, "active:", isActive, "locked:", isLocked)
                                    if (!isDownloaded)
                                        whisperModelManager.downloadModel(modelData.id)
                                    else
                                        whisperModelManager.setActiveModel(modelData.id)
                                }
                            }
                        }
                    }

                    Text {
                        text: {
                            var mb = modelData.sizeBytes / 1000000
                            if (mb >= 1000) return (mb / 1000).toFixed(1) + " GB"
                            return Math.round(mb) + " MB"
                        }
                        font.pixelSize: Typography.fontSizeSmall
                        color: Colors.textSecondary
                        leftPadding: 28
                    }

                    Row {
                        spacing: Theme.spacingSmall
                        leftPadding: 28
                        Text { text: qsTr("Speed"); font.pixelSize: Typography.fontSizeSmall; color: Colors.textSecondary; width: 80 }
                        Rectangle {
                            width: 120; height: 8; radius: 4
                            color: Colors.withAlpha(Colors.accent, 0.15)
                            anchors.verticalCenter: parent.verticalCenter
                            Rectangle { width: parent.width * (modelData.speedPercent ?? 0) / 100; height: parent.height; radius: 4; color: Colors.accent }
                        }
                        Text { text: (modelData.speedPercent ?? 0) + "%"; font.pixelSize: Typography.fontSizeSmall; color: Colors.textSecondary }
                    }

                    Row {
                        spacing: Theme.spacingSmall
                        leftPadding: 28
                        Text { text: qsTr("Accuracy"); font.pixelSize: Typography.fontSizeSmall; color: Colors.textSecondary; width: 80 }
                        Rectangle {
                            width: 120; height: 8; radius: 4
                            color: Colors.withAlpha(Colors.warning, 0.15)
                            anchors.verticalCenter: parent.verticalCenter
                            Rectangle { width: parent.width * (modelData.accuracyPercent ?? 0) / 100; height: parent.height; radius: 4; color: Colors.warning }
                        }
                        Text { text: (modelData.accuracyPercent ?? 0) + "%"; font.pixelSize: Typography.fontSizeSmall; color: Colors.textSecondary }
                    }

                    Text {
                        visible: (modelData.description ?? "").length > 0
                        text: modelData.description ?? ""
                        font.pixelSize: Typography.fontSizeSmall
                        color: Colors.textSecondary
                        leftPadding: 28
                        wrapMode: Text.WordWrap
                        width: parent.width - 28
                    }

                    Text {
                        visible: (modelData.unavailableReason ?? "").length > 0
                        text: modelData.unavailableReason ?? ""
                        font.pixelSize: Typography.fontSizeSmall
                        color: Colors.warning
                        leftPadding: 28
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    z: -1
                    enabled: isDownloaded && !isActive && !isLocked
                    cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: whisperModelManager.setActiveModel(modelData.id)
                }
            }
        }

        // Download progress bar
        Column {
            width: parent.width
            spacing: 4
            visible: whisperModelManager?.isDownloading ?? false

            Rectangle {
                width: parent.width; height: 6; radius: 3
                color: Colors.withAlpha(Colors.textSecondary, 0.2)
                Rectangle {
                    width: parent.width * (whisperModelManager?.downloadProgress ?? 0)
                    height: parent.height; radius: 3; color: Colors.accent
                    Behavior on width { NumberAnimation { duration: Theme.durationNormal } }
                }
            }

            RowLayout {
                width: parent.width
                Text {
                    text: qsTr("Downloading... %1%").arg(Math.round((whisperModelManager?.downloadProgress ?? 0) * 100))
                    font.pixelSize: Typography.fontSizeSmall; color: Colors.textSecondary
                }
                Item { Layout.fillWidth: true }
                Text {
                    text: qsTr("Cancel"); font.pixelSize: Typography.fontSizeSmall; color: Colors.error
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: whisperModelManager.cancelDownload() }
                }
            }
        }

        Text {
            text: qsTr("Larger models deliver better results but take longer.")
            font.pixelSize: Typography.fontSizeSmall
            color: Colors.withAlpha(Colors.textSecondary, 0.6)
        }

        Text {
            visible: (whisperModelManager?.totalDiskUsage() ?? 0) > 0
            text: {
                var mb = (whisperModelManager?.totalDiskUsage() ?? 0) / 1000000
                if (mb >= 1000) return qsTr("Disk usage: %1 GB").arg((mb / 1000).toFixed(1))
                return qsTr("Disk usage: %1 MB").arg(Math.round(mb))
            }
            font.pixelSize: Typography.fontSizeSmall; color: Colors.textSecondary
        }
    }

    // ── Advanced Settings (GPU, Threads, Language) ──
    Column {
        width: parent.width
        spacing: Theme.spacingSmall

        Rectangle { width: parent.width; height: 1; color: Colors.withAlpha(Colors.border, 0.3) }

        Text {
            text: qsTr("Advanced")
            font.pixelSize: Typography.fontSizeMedium
            font.weight: Font.DemiBold
            color: Colors.textPrimary
        }

        // GPU Status Badge
        Row {
            spacing: Theme.spacingSmall
            Rectangle {
                width: gpuRow.implicitWidth + Theme.spacingSmall * 2
                height: 24; radius: Theme.radiusSmall
                color: (whisperModelManager?.gpuAccelerated ?? false)
                       ? Colors.withAlpha(Colors.success, 0.15)
                       : Colors.withAlpha(Colors.textSecondary, 0.1)
                border.width: Theme.borderThin
                border.color: (whisperModelManager?.gpuAccelerated ?? false)
                              ? Colors.withAlpha(Colors.success, 0.3) : Colors.border
                Row {
                    id: gpuRow; anchors.centerIn: parent; spacing: 4
                    Text { text: (whisperModelManager?.gpuAccelerated ?? false) ? "⚡" : "🐢"; font.pixelSize: Typography.fontSizeSmall }
                    Text { text: whisperModelManager?.gpuBackend ?? "CPU"; font.pixelSize: Typography.fontSizeSmall; color: (whisperModelManager?.gpuAccelerated ?? false) ? Colors.success : Colors.textSecondary }
                }
            }
        }

        // Auto-detect language
        RowLayout {
            width: parent.width; spacing: Theme.spacingSmall
            Column {
                Layout.fillWidth: true
                Text { text: qsTr("Auto-detect language"); font.pixelSize: Typography.fontSizeSmall; color: Colors.textPrimary }
                Text { text: qsTr("Automatically detects DE, EN, ES and other languages"); font.pixelSize: Typography.fontSizeXSmall; color: Colors.textSecondary }
            }
            Switch {
                checked: whisperModelManager?.autoDetectLanguage ?? true
                onToggled: whisperModelManager.setAutoDetectLanguage(checked)
            }
        }

        // Language selector (visible when auto-detect is off)
        RowLayout {
            width: parent.width; spacing: Theme.spacingSmall
            visible: !(whisperModelManager?.autoDetectLanguage ?? true)

            Column {
                Layout.fillWidth: true
                Text { text: qsTr("Transcription language"); font.pixelSize: Typography.fontSizeSmall; color: Colors.textPrimary }
                Text { text: qsTr("Select the language you will be speaking"); font.pixelSize: Typography.fontSizeXSmall; color: Colors.textSecondary }
            }

            ComboBox {
                id: langCombo
                Layout.preferredWidth: 140
                model: [
                    { code: "de", label: "Deutsch" },
                    { code: "en", label: "English" },
                    { code: "es", label: "Español" },
                    { code: "fr", label: "Français" },
                    { code: "it", label: "Italiano" },
                    { code: "pt", label: "Português" },
                    { code: "nl", label: "Nederlands" },
                    { code: "pl", label: "Polski" },
                    { code: "ru", label: "Русский" },
                    { code: "ja", label: "日本語" },
                    { code: "zh", label: "中文" },
                    { code: "ko", label: "한국어" }
                ]
                textRole: "label"
                valueRole: "code"
                currentIndex: {
                    var sel = whisperModelManager?.selectedLanguage ?? "de"
                    for (var i = 0; i < model.length; i++) {
                        if (model[i].code === sel) return i
                    }
                    return 0
                }
                onActivated: whisperModelManager.setSelectedLanguage(currentValue)

                background: Rectangle {
                    radius: Theme.radiusSmall
                    color: Colors.withAlpha(Colors.surface, 0.6)
                    border.width: Theme.borderThin
                    border.color: Colors.border
                }
                contentItem: Text {
                    text: langCombo.displayText
                    font.pixelSize: Typography.fontSizeSmall
                    color: Colors.textPrimary
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: Theme.spacingSmall
                }
            }
        }
    }
}
