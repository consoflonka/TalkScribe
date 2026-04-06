pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../styles"

/**
 * Text Snippets settings — trigger phrases that expand into replacement text.
 * E.g., saying "E-Mail Conso" inserts "conso.flonka@yahoo.de".
 */
Column {
    id: root
    spacing: Theme.spacingMedium
    width: parent ? parent.width : 400

    property var snippetList: textPostProcessor.snippetsList()

    Connections {
        target: textPostProcessor
        function onSnippetsChanged() {
            root.snippetList = textPostProcessor.snippetsList()
        }
    }

    // ── Section Header ──
    Text {
        text: qsTr("Text Snippets")
        font.pixelSize: Typography.fontSizeLarge
        font.weight: Font.DemiBold
        color: Colors.textPrimary
    }

    Text {
        text: qsTr("Define trigger phrases that expand into longer text during dictation. When you say the trigger, it gets replaced by the full text.")
        font.pixelSize: Typography.fontSizeSmall
        color: Colors.textSecondary
        wrapMode: Text.WordWrap
        width: parent.width
    }

    // ── Snippet List ──
    Column {
        width: parent.width
        spacing: Theme.spacingSmall

        Repeater {
            model: root.snippetList

            Rectangle {
                id: snippetCard
                required property int index
                required property var modelData

                width: root.width
                height: snippetCard.editing ? editCol.implicitHeight + Theme.spacingMedium * 2
                                           : snippetRow.implicitHeight + Theme.spacingMedium * 2
                radius: Theme.radiusMedium
                color: Colors.withAlpha(Colors.surface, 0.4)
                border.width: Theme.borderThin
                border.color: Colors.border

                property bool editing: false

                // ── Display Mode ──
                RowLayout {
                    id: snippetRow
                    anchors.fill: parent
                    anchors.margins: Theme.spacingMedium
                    spacing: Theme.spacingSmall
                    visible: !snippetCard.editing

                    Column {
                        Layout.fillWidth: true
                        spacing: Theme.spacingXXS

                        Text {
                            text: "\u201C" + (snippetCard.modelData.trigger ?? "") + "\u201D"
                            font.pixelSize: Typography.fontSizeSmall
                            font.weight: Font.Medium
                            color: Colors.textPrimary
                            elide: Text.ElideRight
                            width: parent.width
                        }
                        Text {
                            text: "\u2192 " + (snippetCard.modelData.text ?? "")
                            font.pixelSize: Typography.fontSizeXSmall
                            color: Colors.textSecondary
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }

                    // Edit button
                    Text {
                        text: "\u270E" // pencil
                        font.pixelSize: Typography.fontSizeMedium
                        color: editArea.containsMouse ? Colors.accent : Colors.textSecondary
                        Behavior on color { ColorAnimation { duration: Theme.durationFast } }
                        MouseArea {
                            id: editArea
                            anchors.fill: parent
                            anchors.margins: -4
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                editTriggerField.text = snippetCard.modelData.trigger ?? ""
                                editTextField.text = snippetCard.modelData.text ?? ""
                                snippetCard.editing = true
                            }
                        }
                    }

                    // Delete button
                    Text {
                        text: "\u00D7"
                        font.pixelSize: Typography.fontSizeLarge
                        color: deleteArea.containsMouse ? Colors.error : Colors.textSecondary
                        Behavior on color { ColorAnimation { duration: Theme.durationFast } }
                        MouseArea {
                            id: deleteArea
                            anchors.fill: parent
                            anchors.margins: -4
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: textPostProcessor.removeSnippet(snippetCard.index)
                        }
                    }
                }

                // ── Edit Mode ──
                ColumnLayout {
                    id: editCol
                    anchors.fill: parent
                    anchors.margins: Theme.spacingMedium
                    spacing: Theme.spacingSmall
                    visible: snippetCard.editing

                    TextField {
                        id: editTriggerField
                        Layout.fillWidth: true
                        placeholderText: qsTr("Trigger phrase...")
                        font.pixelSize: Typography.fontSizeSmall
                        color: Colors.textPrimary
                        placeholderTextColor: Colors.textTertiary
                        background: Rectangle {
                            radius: Theme.radiusSmall
                            color: Colors.withAlpha(Colors.surface, 0.6)
                            border.width: Theme.borderThin
                            border.color: editTriggerField.activeFocus ? Colors.accent : Colors.border
                        }
                    }

                    TextField {
                        id: editTextField
                        Layout.fillWidth: true
                        placeholderText: qsTr("Replacement text...")
                        font.pixelSize: Typography.fontSizeSmall
                        color: Colors.textPrimary
                        placeholderTextColor: Colors.textTertiary
                        background: Rectangle {
                            radius: Theme.radiusSmall
                            color: Colors.withAlpha(Colors.surface, 0.6)
                            border.width: Theme.borderThin
                            border.color: editTextField.activeFocus ? Colors.accent : Colors.border
                        }
                    }

                    RowLayout {
                        spacing: Theme.spacingSmall

                        Rectangle {
                            width: Math.max(80, saveBtnText.implicitWidth + Theme.spacingMedium * 2)
                            height: Theme.buttonHeightSmall
                            radius: Theme.radiusSmall
                            color: saveBtnArea.pressed ? Colors.withAlpha(Colors.accent, 0.3)
                                   : saveBtnArea.containsMouse ? Colors.withAlpha(Colors.accent, 0.2)
                                   : Colors.withAlpha(Colors.accent, 0.1)
                            Behavior on color { ColorAnimation { duration: Theme.durationFast } }
                            Text {
                                id: saveBtnText
                                anchors.centerIn: parent
                                text: qsTr("Save")
                                font.pixelSize: Typography.fontSizeSmall
                                font.weight: Font.Medium
                                color: Colors.accent
                            }
                            MouseArea {
                                id: saveBtnArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (editTriggerField.text.trim().length > 0) {
                                        textPostProcessor.updateSnippet(snippetCard.index,
                                            editTriggerField.text.trim(), editTextField.text)
                                        snippetCard.editing = false
                                    }
                                }
                            }
                        }

                        Rectangle {
                            width: Math.max(80, cancelBtnText.implicitWidth + Theme.spacingMedium * 2)
                            height: Theme.buttonHeightSmall
                            radius: Theme.radiusSmall
                            color: cancelBtnArea.pressed ? Colors.withAlpha(Colors.textSecondary, 0.15)
                                   : cancelBtnArea.containsMouse ? Colors.withAlpha(Colors.textSecondary, 0.1)
                                   : "transparent"
                            Behavior on color { ColorAnimation { duration: Theme.durationFast } }
                            Text {
                                id: cancelBtnText
                                anchors.centerIn: parent
                                text: qsTr("Cancel")
                                font.pixelSize: Typography.fontSizeSmall
                                color: Colors.textSecondary
                            }
                            MouseArea {
                                id: cancelBtnArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: snippetCard.editing = false
                            }
                        }
                    }
                }
            }
        }
    }

    // ── Add New Snippet ──
    Rectangle {
        id: addCard
        width: parent.width
        height: addCard.expanded ? addCol.implicitHeight + Theme.spacingMedium * 2
                                : Theme.buttonHeightMedium
        radius: Theme.radiusMedium
        color: Colors.withAlpha(Colors.surface, 0.4)
        border.width: Theme.borderThin
        border.color: Colors.border

        property bool expanded: false

        Behavior on height { NumberAnimation { duration: Theme.durationNormal; easing.type: Easing.OutCubic } }

        // Collapsed: "+" button
        Text {
            anchors.centerIn: parent
            text: qsTr("+ New Snippet")
            font.pixelSize: Typography.fontSizeSmall
            font.weight: Font.Medium
            color: Colors.accent
            visible: !addCard.expanded
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            visible: !addCard.expanded
            onClicked: {
                addCard.expanded = true
                newTriggerField.forceActiveFocus()
            }
        }

        // Expanded: form
        ColumnLayout {
            id: addCol
            anchors.fill: parent
            anchors.margins: Theme.spacingMedium
            spacing: Theme.spacingSmall
            visible: addCard.expanded

            TextField {
                id: newTriggerField
                Layout.fillWidth: true
                placeholderText: qsTr("Trigger phrase (e.g. \"E-Mail Conso\")")
                font.pixelSize: Typography.fontSizeSmall
                color: Colors.textPrimary
                placeholderTextColor: Colors.textTertiary
                background: Rectangle {
                    radius: Theme.radiusSmall
                    color: Colors.withAlpha(Colors.surface, 0.6)
                    border.width: Theme.borderThin
                    border.color: newTriggerField.activeFocus ? Colors.accent : Colors.border
                }
            }

            TextField {
                id: newTextField
                Layout.fillWidth: true
                placeholderText: qsTr("Replacement text (e.g. \"conso.flonka@yahoo.de\")")
                font.pixelSize: Typography.fontSizeSmall
                color: Colors.textPrimary
                placeholderTextColor: Colors.textTertiary
                background: Rectangle {
                    radius: Theme.radiusSmall
                    color: Colors.withAlpha(Colors.surface, 0.6)
                    border.width: Theme.borderThin
                    border.color: newTextField.activeFocus ? Colors.accent : Colors.border
                }

                onAccepted: {
                    if (newTriggerField.text.trim().length > 0) {
                        textPostProcessor.addSnippet(newTriggerField.text.trim(), newTextField.text)
                        newTriggerField.text = ""
                        newTextField.text = ""
                        addCard.expanded = false
                    }
                }
            }

            RowLayout {
                spacing: Theme.spacingSmall

                Rectangle {
                    width: Math.max(80, addSnipBtnText.implicitWidth + Theme.spacingMedium * 2)
                    height: Theme.buttonHeightSmall
                    radius: Theme.radiusSmall
                    color: addSnipBtnArea.pressed ? Colors.withAlpha(Colors.accent, 0.3)
                           : addSnipBtnArea.containsMouse ? Colors.withAlpha(Colors.accent, 0.2)
                           : Colors.withAlpha(Colors.accent, 0.1)
                    Behavior on color { ColorAnimation { duration: Theme.durationFast } }
                    Text {
                        id: addSnipBtnText
                        anchors.centerIn: parent
                        text: qsTr("Add")
                        font.pixelSize: Typography.fontSizeSmall
                        font.weight: Font.Medium
                        color: Colors.accent
                    }
                    MouseArea {
                        id: addSnipBtnArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (newTriggerField.text.trim().length > 0) {
                                textPostProcessor.addSnippet(newTriggerField.text.trim(), newTextField.text)
                                newTriggerField.text = ""
                                newTextField.text = ""
                                addCard.expanded = false
                            }
                        }
                    }
                }

                Rectangle {
                    width: Math.max(80, addCancelBtnText.implicitWidth + Theme.spacingMedium * 2)
                    height: Theme.buttonHeightSmall
                    radius: Theme.radiusSmall
                    color: addCancelBtnArea.pressed ? Colors.withAlpha(Colors.textSecondary, 0.15)
                           : addCancelBtnArea.containsMouse ? Colors.withAlpha(Colors.textSecondary, 0.1)
                           : "transparent"
                    Behavior on color { ColorAnimation { duration: Theme.durationFast } }
                    Text {
                        id: addCancelBtnText
                        anchors.centerIn: parent
                        text: qsTr("Cancel")
                        font.pixelSize: Typography.fontSizeSmall
                        color: Colors.textSecondary
                    }
                    MouseArea {
                        id: addCancelBtnArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            newTriggerField.text = ""
                            newTextField.text = ""
                            addCard.expanded = false
                        }
                    }
                }
            }
        }
    }
}
