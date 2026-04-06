pragma Singleton
import QtQuick

QtObject {
    // Design Style
    property string designStyle: "modern"

    function setDesignStyle(style) {
        designStyle = style
    }

    // Helper for 3-way lookup
    // "modern-b2b" inherits Modern spacing/sizing; "saas" uses modern spacing with refined radius
    function _v(modern, pro, ent) {
        if (designStyle === "modern-b2b" || designStyle === "saas") return modern
        if (designStyle === "enterprise") return ent
        if (designStyle === "professional") return pro
        return modern
    }

    // Spacing (Enterprise is more compact; saas uses modern/generous spacing)
    readonly property int spacingXXS: 4
    property int spacingXS:     _v(8, 8, 6)
    property int spacingSmall:  _v(12, 12, 10)
    property int spacingMedium: _v(16, 16, 12)
    property int spacingLarge:  _v(20, 20, 16)
    property int spacingXL:     _v(24, 24, 20)
    property int spacingXXL:    _v(32, 32, 24)
    property int spacingHuge:   _v(48, 48, 36)

    // Border Radius
    // saas: refined 8/12px (professional feel with generous rounding)
    // Professional: smaller, Enterprise: nearly square
    property int radiusSmall:  designStyle === "saas" ? 6  : _v(4, 4, 2)
    property int radiusMedium: designStyle === "saas" ? 8  : _v(8, 6, 4)
    property int radiusLarge:  designStyle === "saas" ? 12 : _v(12, 8, 6)
    property int radiusXL:     designStyle === "saas" ? 16 : _v(16, 12, 8)
    readonly property int radiusRound: 999

    // Border Width
    readonly property int borderThin: 1
    readonly property int borderMedium: 2
    readonly property int borderThick: 3

    // Shadows (saas has more pronounced card depth)
    readonly property int shadowSmall: 2
    readonly property int shadowMedium: 4
    property int shadowLarge: designStyle === "saas" ? 12 : 8

    // Animation Durations (ms) - Enterprise: faster/snappier; saas: responsive but smooth
    property int durationFast:   designStyle === "saas" ? 150 : _v(150, 100, 80)
    property int durationNormal: designStyle === "saas" ? 200 : _v(200, 150, 100)
    property int durationSlow:   designStyle === "saas" ? 250 : _v(300, 200, 150)

    // Hover scale amount - saas: no scale (professional); modern-b2b: subtle 2%
    property real hoverScaleAmount: designStyle === "saas" ? 1.0
        : (designStyle === "modern-b2b" ? 1.02 : _v(1.06, 1.0, 1.0))

    // Component Sizes (Enterprise: more compact; saas: comfortable like modern)
    property int buttonHeightSmall:  _v(32, 32, 28)
    property int buttonHeightMedium: _v(40, 40, 36)
    property int buttonHeightLarge:  _v(48, 48, 42)

    property int inputHeight:  _v(40, 40, 36)
    readonly property int sidebarWidth: 250
    property int headerHeight: _v(60, 60, 52)

    // Z-Index Layers
    readonly property int zIndexBase: 0
    readonly property int zIndexDropdown: 100
    readonly property int zIndexModal: 200
    readonly property int zIndexTooltip: 300
    readonly property int zIndexOverlay: 500
    readonly property int zIndexCalibration: 950
    readonly property int zIndexLockout: 99999
}
