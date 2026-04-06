pragma Singleton
import QtQuick

QtObject {
    // Theme & Design Style properties
    property string currentTheme: "dark"
    property string designStyle: "modern" // "modern", "modern-b2b", "professional", "enterprise", "saas"

    // Helper: 6-way lookup (modernLight, modernDark, proLight, proDark, entLight, entDark)
    // "modern-b2b" inherits Modern palette (same colors, different component behavior)
    // "saas" uses professional palette (light = pro-light, dark = pro-dark)
    function _c(ml, md, pl, pd, el, ed) {
        if (designStyle === "saas") return currentTheme === "light" ? pl : pd
        if (designStyle === "modern-b2b")
            return currentTheme === "light" ? ml : md
        if (designStyle === "enterprise")
            return currentTheme === "light" ? el : ed
        if (designStyle === "professional")
            return currentTheme === "light" ? pl : pd
        return currentTheme === "light" ? ml : md
    }

    // White-label branding override: if org has custom accent color, use it
    property string _brandAccent: (typeof licenseClient !== "undefined" && licenseClient && licenseClient.orgAccentColor)
        ? licenseClient.orgAccentColor : ""

    // User-chosen accent from settings (set via setAccent(), does not break binding)
    property string _userAccent: ""

    // Primary Colors — priority: Org Branding > User Setting > Design-Style Default
    property color accent: _brandAccent.length > 0 ? _brandAccent
        : (_userAccent.length > 0 ? _userAccent
        : (designStyle === "saas" ? (currentTheme === "light" ? "#2563EB" : "#3B82F6") : "#3B82F6"))
    property color primary: accent
    property color primaryDark: Qt.darker(accent, 1.3)
    property color primaryLight: Qt.lighter(accent, 1.3)
    // Dynamic: dark text on light accent, white text on dark accent (WCAG 2.1)
    function _luminance(c) {
        return 0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b
    }
    // Generic helper: returns black or white text for any background color
    function contrastText(bg) {
        return _luminance(bg) > 0.45 ? "#0F172A" : "#FFFFFF"
    }
    property color textOnPrimary: contrastText(accent)
    property color textOnError: contrastText(error)
    property color textOnWarning: contrastText(warning)
    property color textOnSuccess: contrastText(success)

    // White-label branding override for secondary color
    property string _brandSecondary: (typeof licenseClient !== "undefined" && licenseClient && licenseClient.orgSecondaryColor)
        ? licenseClient.orgSecondaryColor : ""

    // Secondary & Functional Colors
    property color secondary: _brandSecondary.length > 0 ? _brandSecondary : "#EC4899"
    property color success: (designStyle === "saas" && currentTheme === "light") ? "#059669"
        : (currentTheme === "light" ? "#059669" : "#22C55E")
    property color warning: currentTheme === "light" ? "#B45309" : "#F59E0B"
    property color error: (designStyle === "saas" && currentTheme === "light") ? "#DC2626" : "#EF4444"
    property color errorDark: Qt.darker(error, 1.2)
    property color errorLight: Qt.lighter(error, 1.2)
    property color info: "#3B82F6"
    property color cyan: "#06B6D4"

    // Backgrounds
    property color background:      designStyle === "saas" ? (currentTheme === "light" ? "#F8FAFC" : "#0F172A")
        : _c("#F1F5F9", "#0A0A0B", "#FFFFFF", "#0F172A", "#F8F9FA", "#151820")
    property color backgroundDark:  designStyle === "saas" ? (currentTheme === "light" ? "#F1F5F9" : "#020617")
        : _c("#E2E8F0", "#050506", "#F3F4F6", "#020617", "#EFF0F3", "#0E1117")
    property color surface:         designStyle === "saas" ? (currentTheme === "light" ? "#FFFFFF" : "#1E293B")
        : _c("#FFFFFF", "#121214", "#FFFFFF", "#1E293B", "#FFFFFF", "#1C2030")
    property color surfaceLight:    designStyle === "saas" ? (currentTheme === "light" ? "#F8FAFC" : "#334155")
        : _c("#E2E8F0", "#1C1C1E", "#E5E7EB", "#334155", "#E0E4EA", "#242838")
    property color surfaceDark:     designStyle === "saas" ? (currentTheme === "light" ? "#F1F5F9" : "#0F172A")
        : _c("#B0BEC5", "#080809", "#D1D5DB", "#0F172A", "#C5CCD4", "#0C0F18")
    property color surfaceHighlight: designStyle === "saas" ? (currentTheme === "light" ? "#EFF6FF" : "#334155")
        : _c("#D5DCE5", "#2A2A2E", "#E5E7EB", "#334155", "#DCE0E8", "#2C3148")

    // Accent Variations
    property color accentLight: Qt.lighter(accent, 1.2)
    property color accentDark: Qt.darker(accent, 1.2)
    property color accentMedium: Qt.darker(accent, 1.1)
    property color accentHover: Qt.lighter(accent, 1.1)

    // Panel & Card Colors
    property color panelBg: designStyle === "saas" ? (currentTheme === "light" ? "#FFFFFF" : "#1E293B")
        : _c("#F8FAFC", "#141416", "#FAFAFA", "#1E293B", "#F9FAFB", "#181C28")
    property color cardBg:  designStyle === "saas" ? (currentTheme === "light" ? "#FFFFFF" : "#1E293B")
        : _c("#F8FAFC", "#1A1A1C", "#FAFAFA", "#1E293B", "#F9FAFB", "#1E2230")
    property color cardBackground: surface
    property color cardBorder: border
    property color badgeBackground: surfaceLight
    property color surfaceElevated: surfaceLight
    property color errorBackground: designStyle === "saas" ? (currentTheme === "light" ? "#FEF2F2" : Qt.rgba(0.94, 0.27, 0.27, 0.1))
        : _c("#FEF2F2", Qt.rgba(0.94, 0.27, 0.27, 0.08), "#FEF2F2", Qt.rgba(0.94, 0.27, 0.27, 0.1), "#FEF2F2", Qt.rgba(0.94, 0.27, 0.27, 0.1))

    // Text Colors
    property color textPrimary:   designStyle === "saas" ? (currentTheme === "light" ? "#0F172A" : "#F1F5F9")
        : _c("#0F172A", "#F8FAFC", "#111827", "#F1F5F9", "#1C1E26", "#DCE1E8")
    property color textSecondary: designStyle === "saas" ? (currentTheme === "light" ? "#64748B" : "#94A3B8")
        : _c("#475569", "#94A3B8", "#6B7280", "#94A3B8", "#6B7280", "#7C879A")
    property color textTertiary:  designStyle === "saas" ? (currentTheme === "light" ? "#64748B" : "#8294AA")
        : _c("#64748B", "#8294AA", "#6B7280", "#8294AA", "#6B7280", "#7A8A9E")
    property color textDisabled:  designStyle === "saas" ? (currentTheme === "light" ? "#7C8FA3" : "#5E7086")
        : _c("#6B7280", "#4A5E72", "#6B7280", "#5E7086", "#7C879A", "#56687C")
    property color textOnAccent: textOnPrimary

    // Borders
    property color border:      designStyle === "saas" ? (currentTheme === "light" ? "#E2E8F0" : "#334155")
        : _c("#E2E8F0", "#27272A", "#E5E7EB", "#334155", "#D1D5DC", "#2A3045")
    property color borderLight: designStyle === "saas" ? (currentTheme === "light" ? "#F1F5F9" : "#475569")
        : _c("#D5DCE5", "#3F3F46", "#D1D5DB", "#475569", "#D1D5DC", "#343A52")
    property color borderDark:  designStyle === "saas" ? (currentTheme === "light" ? "#CBD5E1" : "#1E293B")
        : _c("#CBD5E1", "#18181B", "#D1D5DB", "#1E293B", "#B8BEC8", "#1C2035")

    // Assets
    property url bannerImage: currentTheme === "light" ? "qrc:/qt/qml/TalkLess/resources/images/background_light.png" : "qrc:/qt/qml/TalkLess/resources/images/background.png"
    property url splashImage: currentTheme === "light" ? "qrc:/qt/qml/TalkLess/resources/images/splashScreen_light.png" : "qrc:/qt/qml/TalkLess/resources/images/splashScreen.png"

    // Special Tokens
    property color shadow: (designStyle === "saas" && currentTheme === "light") ? Qt.rgba(15/255, 23/255, 42/255, 0.08)
        : _c("rgba(0,0,0,0.13)", "rgba(0,0,0,0.4)", "rgba(0,0,0,0.08)", "rgba(0,0,0,0.15)", "rgba(0,0,0,0.06)", "rgba(0,0,0,0.2)")
    property color overlay: currentTheme === "light" ? "rgba(0,0,0,0.02)" : "rgba(255,255,255,0.02)"
    property color overlayModal: currentTheme === "light" ? Qt.rgba(0,0,0,0.5) : Qt.rgba(0,0,0,0.65)
    property color overlayHeavy: currentTheme === "light" ? Qt.rgba(0,0,0,0.7) : Qt.rgba(0,0,0,0.8)
    property color white: "#FFFFFF"
    property color black: "#000000"

    // Alpha helper: derive a translucent variant of any color
    function withAlpha(baseColor, alpha) {
        return Qt.rgba(baseColor.r, baseColor.g, baseColor.b, alpha)
    }

    // Premium/Tier Colors (Gold = industry standard for premium badges)
    property color premiumGold: currentTheme === "light" ? "#A0734D" : "#D4A574"
    property color premiumGoldLight: "#F5DEB3"
    property color premiumGoldBg: currentTheme === "light" ? Qt.rgba(0.83, 0.65, 0.46, 0.08) : Qt.rgba(0.83, 0.65, 0.46, 0.12)

    // Business Tier Color
    property color businessTier: "#9966E6"

    // Usage Meter Colors (traffic light system: green → yellow → orange → red)
    property color usageOk: success
    property color usageWarning: "#EAB308"
    property color usageCritical: "#F97316"
    property color usageFull: error

    // Gradients - saas: flat solid (no gradient); Professional & Enterprise: flat solid accent
    property color gradientPrimaryStart: accent
    property color gradientPrimaryEnd: (designStyle === "saas" || designStyle === "professional" || designStyle === "enterprise")
        ? accent
        : (designStyle === "modern"
            ? ((accent == "#3B82F6" || accent == "#3b82f6") ? "#D214FD" : Qt.lighter(accent, 1.5))
            : accent)
    property color gradientBgStart: designStyle === "saas" ? (currentTheme === "light" ? "#F8FAFC" : "#0F172A")
        : _c("#F8FAFC", "#0C0C0E", "#FFFFFF", "#0F172A", "#F8F9FA", "#161A24")
    property color gradientBgEnd:   designStyle === "saas" ? (currentTheme === "light" ? "#F1F5F9" : "#1E293B")
        : _c("#F1F5F9", "#141416", "#F9FAFB", "#1E293B", "#EFF0F3", "#1C2030")

    // Centralized Color Palette (for pickers, category editors, action tiles)
    readonly property var colorPalette: [
        "#EF4444", "#F97316", "#F59E0B", "#22C55E", "#14B8A6", "#0891B2",
        "#2563EB", "#3B82F6", "#6366F1", "#8B5CF6", "#A855F7", "#EC4899",
        "#10B981", "#06B6D4", "#84CC16", "#1E40AF", "#6B7280", "#374151",
        "#1E293B", "#FFFFFF"
    ]

    // Accent color presets for settings
    readonly property var accentPresets: [
        "#3B82F6", "#EF4444", "#D946EF", "#22C55E", "#EAB308", "#06B6D4", "#F97316"
    ]

    // Logic: Self-Update from Service
    function setTheme(theme) {
        currentTheme = theme
    }

    function setAccent(newAccent) {
        _userAccent = newAccent || ""
    }

    function setDesignStyle(style) {
        designStyle = style
    }
}
