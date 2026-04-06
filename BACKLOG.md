# TalkScribe Backlog

## Windows-Port

**Priorität:** Mittel
**Geschätzter Aufwand:** 2-3 Tage
**Status:** Offen

### Beschreibung
TalkScribe für Windows verfügbar machen. Die Tech-Basis (Qt/QML, whisper.cpp, miniaudio, SpeexDSP, CMake) ist komplett cross-platform — nur platform-spezifische Teile müssen angepasst werden.

### Was funktioniert out-of-the-box
- QML-UI (komplett plattformunabhängig)
- whisper.cpp (kompiliert nativ auf Windows)
- miniaudio (Audio-Capture auf Windows supported)
- SpeexDSP (cross-platform)
- CMake Build-System

### Was angepasst werden muss
1. **MacWindowHelper** — Objective-C++ (`MacWindowHelper.mm`) durch Win32-API-Äquivalent ersetzen oder entfernen falls nicht kritisch
2. **CMakeLists.txt** — macOS-Frameworks (Carbon, CoreAudio, AVFoundation) durch Windows-Libs ersetzen. Pattern aus TalkLess übernehmen
3. **Globale Hotkeys** — Falls genutzt: Windows RegisterHotKey API
4. **Build-Setup** — vcpkg Dependencies, MSVC/MinGW Konfiguration (aus TalkLess übernehmen)
5. **Installer** — NSIS oder WiX für Windows-Distribution
6. **CI-Pipeline** — GitHub Actions für Windows-Build

### Referenz
TalkLess hat bereits einen funktionierenden Windows-Build mit vcpkg — CMake-Config, Dependencies und CI können als Vorlage dienen.
