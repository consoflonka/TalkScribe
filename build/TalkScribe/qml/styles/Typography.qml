pragma Singleton
import QtQuick

QtObject {
    // Design Style
    property string designStyle: "modern"

    function setDesignStyle(style) {
        designStyle = style
    }

    // Font Families (with cross-platform fallback chains)
    // Modern = "Segoe UI", modern-b2b/Professional = "Inter", Enterprise = "Segoe UI", saas = "Plus Jakarta Sans"
    property string fontFamily: designStyle === "saas" ? "Plus Jakarta Sans, Inter, Segoe UI, sans-serif"
        : ((designStyle === "professional" || designStyle === "modern-b2b") ? "Inter, Segoe UI, Helvetica Neue, sans-serif" : "Segoe UI, -apple-system, Helvetica Neue, sans-serif")

    // Display Font (headings, sidebar logo text)
    // Modern = "Outfit", modern-b2b/Professional = "Inter", Enterprise = "Segoe UI", saas = "Plus Jakarta Sans"
    property string fontFamilyDisplay: designStyle === "saas" ? "Plus Jakarta Sans, Inter, Segoe UI, sans-serif"
        : (designStyle === "modern" ? "Outfit, Segoe UI, sans-serif"
        : ((designStyle === "professional" || designStyle === "modern-b2b") ? "Inter, Segoe UI, Helvetica Neue, sans-serif" : "Segoe UI, -apple-system, Helvetica Neue, sans-serif"))

    // Navigation Font (sidebar items, settings panels)
    // Modern = "Poppins", modern-b2b/Professional = "Inter", Enterprise = "Segoe UI", saas = "Plus Jakarta Sans"
    property string fontFamilyNav: designStyle === "saas" ? "Plus Jakarta Sans, Inter, Segoe UI, sans-serif"
        : (designStyle === "modern" ? "Poppins, Segoe UI, sans-serif"
        : ((designStyle === "professional" || designStyle === "modern-b2b") ? "Inter, Segoe UI, Helvetica Neue, sans-serif" : "Segoe UI, -apple-system, Helvetica Neue, sans-serif"))

    // Monospace
    readonly property string fontFamilyMono: "Consolas, Monaco, Courier New, monospace"

    // Font Sizes (Enterprise: slightly smaller headings for data density; saas: comfortable like modern)
    property int fontSizeDisplay: designStyle === "enterprise" ? 40 : 48
    property int fontSizeXXL:    designStyle === "enterprise" ? 28 : 32
    property int fontSizeXL:     designStyle === "enterprise" ? 22 : 24
    property int fontSizeLarge:  designStyle === "enterprise" ? 18 : 20
    property int fontSizeMedium: designStyle === "enterprise" ? 15 : 16
    readonly property int fontSizeNormal: 14
    readonly property int fontSizeSmall: 13
    readonly property int fontSizeXSmall: 12
    readonly property int fontSizeCaption: 12
    readonly property int fontSizeTiny: 10

    // Font Weights
    readonly property int fontWeightLight: Font.Light
    readonly property int fontWeightNormal: Font.Normal
    readonly property int fontWeightMedium: Font.Medium
    readonly property int fontWeightBold: Font.Bold

    // Line Heights (multipliers)
    readonly property real lineHeightTight: 1.2
    readonly property real lineHeightNormal: 1.5
    readonly property real lineHeightRelaxed: 1.75

    // Letter Spacing
    readonly property real letterSpacingTight: -0.5
    readonly property real letterSpacingNormal: 0
    readonly property real letterSpacingWide: 0.5
}
