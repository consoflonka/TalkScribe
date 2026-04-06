pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../styles"

/**
 * Dictation statistics section — total words, time saved, transcription count,
 * words-per-day chart, and language breakdown. Supports time-range filtering.
 */
ColumnLayout {
    id: root
    spacing: Theme.spacingMedium

    // Filter: 0 = All time, 1 = Last 30 days, 2 = Last 7 days
    property int filterPeriod: 0

    // ── Filtered data ──
    readonly property var filteredDaily: {
        var data = dictationStatsService.dailyStats
        if (!data || data.length === 0) return []
        if (filterPeriod === 0) return data

        var now = new Date()
        var days = filterPeriod === 1 ? 30 : 7
        var cutoff = new Date(now.getFullYear(), now.getMonth(), now.getDate() - days)
        var cutoffStr = cutoff.toISOString().substring(0, 10)

        var result = []
        for (var i = 0; i < data.length; i++) {
            if ((data[i].date || "") >= cutoffStr)
                result.push(data[i])
        }
        return result
    }

    readonly property int filteredWords: {
        var sum = 0
        for (var i = 0; i < filteredDaily.length; i++)
            sum += (filteredDaily[i].words || 0)
        return sum
    }

    readonly property int filteredTranscriptions: {
        var sum = 0
        for (var i = 0; i < filteredDaily.length; i++)
            sum += (filteredDaily[i].transcriptions || 0)
        return sum
    }

    readonly property string filteredTimeSaved: {
        var savedSecs = Math.floor(filteredWords * 0.4)
        var h = Math.floor(savedSecs / 3600)
        var m = Math.floor((savedSecs % 3600) / 60)
        var s = savedSecs % 60
        if (h > 0) return h + "h " + m + "m"
        if (m > 0) return m + "m " + s + "s"
        return s + "s"
    }

    readonly property var filteredLanguages: {
        var langs = {}
        for (var i = 0; i < filteredDaily.length; i++) {
            var dayLangs = filteredDaily[i].languages
            if (dayLangs) {
                var keys = Object.keys(dayLangs)
                for (var k = 0; k < keys.length; k++) {
                    var lang = keys[k]
                    langs[lang] = (langs[lang] || 0) + dayLangs[keys[k]]
                }
            }
        }
        // Fallback for old data without per-day languages: use global breakdown in "all time" mode
        if (Object.keys(langs).length === 0 && filterPeriod === 0) {
            return dictationStatsService.languageBreakdown
        }
        return langs
    }

    // ── Section Header + Filter ──
    RowLayout {
        Layout.fillWidth: true
        spacing: Theme.spacingSmall

        Text {
            text: qsTr("Dictation")
            font.pixelSize: Typography.fontSizeLarge
            font.weight: Font.DemiBold
            color: Colors.textPrimary
        }
        Item { Layout.fillWidth: true }

        // Filter chips
        Row {
            spacing: 4

            Repeater {
                model: [
                    { label: qsTr("All time"), value: 0 },
                    { label: qsTr("Last 30 days"), value: 1 },
                    { label: qsTr("Last 7 days"), value: 2 }
                ]

                Rectangle {
                    required property var modelData
                    required property int index
                    width: chipText.implicitWidth + 16
                    height: 28
                    radius: 14
                    color: root.filterPeriod === modelData.value
                        ? Colors.accent
                        : Colors.withAlpha(Colors.surface, 0.6)
                    border.width: root.filterPeriod === modelData.value ? 0 : Theme.borderThin
                    border.color: Colors.border

                    Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                    Text {
                        id: chipText
                        anchors.centerIn: parent
                        text: modelData.label
                        font.pixelSize: Typography.fontSizeXSmall
                        font.weight: root.filterPeriod === modelData.value ? Font.DemiBold : Font.Normal
                        color: root.filterPeriod === modelData.value
                            ? Colors.textOnAccent
                            : Colors.textSecondary
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.filterPeriod = modelData.value
                    }
                }
            }
        }
    }

    // ── KPI Cards ──
    RowLayout {
        Layout.fillWidth: true
        spacing: Theme.spacingSmall

        // Words total
        Rectangle {
            Layout.fillWidth: true
            height: 90
            radius: Theme.radiusMedium
            color: Colors.withAlpha(Colors.surface, 0.4)
            border.width: Theme.borderThin
            border.color: Colors.border

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 2

                Text {
                    text: "Abc"
                    font.pixelSize: Typography.fontSizeSmall
                    font.weight: Font.Medium
                    color: Colors.accent
                    Layout.alignment: Qt.AlignHCenter
                }
                Text {
                    text: root.filteredWords.toLocaleString()
                    font.pixelSize: 24
                    font.weight: Font.Bold
                    color: Colors.textPrimary
                    Layout.alignment: Qt.AlignHCenter
                }
                Text {
                    text: qsTr("Words total")
                    font.pixelSize: Typography.fontSizeXSmall
                    color: Colors.textSecondary
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        // Time saved
        Rectangle {
            Layout.fillWidth: true
            height: 90
            radius: Theme.radiusMedium
            color: Colors.withAlpha(Colors.surface, 0.4)
            border.width: Theme.borderThin
            border.color: Colors.border

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 2

                Text {
                    text: "\u23F1"
                    font.pixelSize: Typography.fontSizeSmall
                    Layout.alignment: Qt.AlignHCenter
                }
                Text {
                    text: root.filteredTimeSaved
                    font.pixelSize: 24
                    font.weight: Font.Bold
                    color: Colors.textPrimary
                    Layout.alignment: Qt.AlignHCenter
                }
                Text {
                    text: qsTr("Time saved")
                    font.pixelSize: Typography.fontSizeXSmall
                    color: Colors.textSecondary
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        // Transcriptions
        Rectangle {
            Layout.fillWidth: true
            height: 90
            radius: Theme.radiusMedium
            color: Colors.withAlpha(Colors.surface, 0.4)
            border.width: Theme.borderThin
            border.color: Colors.border

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 2

                Text {
                    text: "\uD83D\uDCAC"
                    font.pixelSize: Typography.fontSizeSmall
                    Layout.alignment: Qt.AlignHCenter
                }
                Text {
                    text: root.filteredTranscriptions.toString()
                    font.pixelSize: 24
                    font.weight: Font.Bold
                    color: Colors.textPrimary
                    Layout.alignment: Qt.AlignHCenter
                }
                Text {
                    text: qsTr("Transcriptions")
                    font.pixelSize: Typography.fontSizeXSmall
                    color: Colors.textSecondary
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }

    // ── Words per Day Chart ──
    Rectangle {
        Layout.fillWidth: true
        height: 200
        radius: Theme.radiusMedium
        color: Colors.withAlpha(Colors.surface, 0.4)
        border.width: Theme.borderThin
        border.color: Colors.border

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Theme.spacingMedium
            spacing: Theme.spacingSmall

            Text {
                text: qsTr("Words per Day")
                font.pixelSize: Typography.fontSizeMedium
                font.weight: Font.Medium
                color: Colors.textPrimary
            }

            // Bar chart
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                readonly property var chartData: {
                    var data = root.filteredDaily
                    if (!data || data.length === 0) return []
                    // Last 14 entries max for readability
                    return data.slice(-14)
                }

                readonly property int maxWords: {
                    var max = 0
                    for (var i = 0; i < chartData.length; i++) {
                        var w = chartData[i].words || 0
                        if (w > max) max = w
                    }
                    return Math.max(max, 1)
                }

                Row {
                    anchors.fill: parent
                    anchors.bottomMargin: 16
                    spacing: 2

                    Repeater {
                        model: parent.parent.chartData.length

                        Rectangle {
                            required property int index
                            readonly property var dayData: parent.parent.parent.chartData[index] || {}
                            readonly property int words: dayData.words || 0

                            width: (parent.width - (parent.parent.parent.chartData.length - 1) * 2) / Math.max(parent.parent.parent.chartData.length, 1)
                            anchors.bottom: parent.bottom
                            height: Math.max(2, (words / parent.parent.parent.maxWords) * parent.height)
                            radius: Theme.radiusSmall
                            color: Colors.accent
                            opacity: 0.8

                            ToolTip.visible: barMa.containsMouse
                            ToolTip.text: (dayData.date || "") + ": " + words + " " + qsTr("words")

                            MouseArea {
                                id: barMa
                                anchors.fill: parent
                                hoverEnabled: true
                                acceptedButtons: Qt.NoButton
                            }
                        }
                    }
                }

                // Date labels (first and last)
                RowLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom

                    Text {
                        readonly property var firstDay: parent.parent.chartData.length > 0 ? parent.parent.chartData[0] : null
                        text: firstDay ? (firstDay.date || "").substring(5) : ""
                        font.pixelSize: Typography.fontSizeXSmall
                        color: Colors.textTertiary
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        readonly property var lastDay: parent.parent.chartData.length > 0 ? parent.parent.chartData[parent.parent.chartData.length - 1] : null
                        text: lastDay ? (lastDay.date || "").substring(5) : ""
                        font.pixelSize: Typography.fontSizeXSmall
                        color: Colors.textTertiary
                    }
                }
            }
        }
    }

    // ── Language Breakdown ──
    Rectangle {
        Layout.fillWidth: true
        visible: {
            var lb = root.filteredLanguages
            return lb && Object.keys(lb).length > 0
        }
        height: langCol.implicitHeight + Theme.spacingMedium * 2
        radius: Theme.radiusMedium
        color: Colors.withAlpha(Colors.surface, 0.4)
        border.width: Theme.borderThin
        border.color: Colors.border

        ColumnLayout {
            id: langCol
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: Theme.spacingMedium
            spacing: Theme.spacingSmall

            Text {
                text: qsTr("Languages")
                font.pixelSize: Typography.fontSizeMedium
                font.weight: Font.Medium
                color: Colors.textPrimary
            }

            Repeater {
                model: {
                    var lb = root.filteredLanguages
                    if (!lb) return []
                    var keys = Object.keys(lb)
                    var result = []
                    for (var i = 0; i < keys.length; i++)
                        result.push({lang: keys[i].toUpperCase(), count: lb[keys[i]]})
                    result.sort(function(a, b) { return b.count - a.count })
                    return result
                }

                RowLayout {
                    required property var modelData
                    Layout.fillWidth: true
                    spacing: Theme.spacingSmall

                    Text {
                        text: modelData.lang
                        font.pixelSize: Typography.fontSizeSmall
                        font.weight: Font.Medium
                        color: Colors.textPrimary
                        Layout.preferredWidth: 60
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 8
                        radius: 4
                        color: Colors.withAlpha(Colors.border, 0.3)

                        Rectangle {
                            width: {
                                var total = root.filteredWords
                                if (total <= 0) return 0
                                return parent.width * (modelData.count / total)
                            }
                            height: parent.height
                            radius: 4
                            color: Colors.accent
                        }
                    }

                    Text {
                        text: modelData.count.toString()
                        font.pixelSize: Typography.fontSizeSmall
                        color: Colors.textSecondary
                        Layout.preferredWidth: 40
                        horizontalAlignment: Text.AlignRight
                    }
                }
            }
        }
    }
}
