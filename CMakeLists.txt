cmake_minimum_required(VERSION 3.21)
project(TalkScribe VERSION 0.1.0 LANGUAGES CXX C OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

include(FetchContent)

# ── Qt ──
find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 Network Concurrent)
qt_standard_project_setup(REQUIRES 6.5)

# ── Sources ──
set(TALKSCRIBE_SOURCES
    src/main.cpp
    src/services/AudioCapture.h
    src/services/AudioCapture.cpp
    src/services/GlobalDictationService.h
    src/services/GlobalDictationService.cpp
    src/services/LocalWhisperProvider.h
    src/services/LocalWhisperProvider.cpp
    src/services/ParakeetProvider.h
    src/services/ParakeetProvider.cpp
    src/services/WhisperModelManager.h
    src/services/WhisperModelManager.cpp
    src/services/TextPostProcessor.h
    src/services/TextPostProcessor.cpp
    src/services/DictationStatsService.h
    src/services/DictationStatsService.cpp
    src/speexResampler.h
    src/speexResampler.cpp
    src/miniaudio_impl.cpp
    src/miniaudio.h
)

# macOS-specific sources
if(APPLE)
    list(APPEND TALKSCRIBE_SOURCES
        src/helpers/MacWindowHelper.h
        src/helpers/MacWindowHelper.mm
    )
endif()

# ── QML Resources ──
set(TALKSCRIBE_QML
    qml/Main.qml
    qml/styles/qmldir
    qml/styles/Colors.qml
    qml/styles/Typography.qml
    qml/styles/Theme.qml
    qml/components/DictationOverlay.qml
    qml/components/DictationSettings.qml
    qml/components/DictationStatsSection.qml
    qml/components/TranscriptionModelSettings.qml
    qml/components/CustomWordsSettings.qml
    qml/components/SnippetSettings.qml
)

qt_add_executable(appTalkScribe ${TALKSCRIBE_SOURCES})

qt_add_qml_module(appTalkScribe
    URI TalkScribe
    VERSION 1.0
    QML_FILES ${TALKSCRIBE_QML}
    RESOURCE_PREFIX /
    NO_RESOURCE_TARGET_PATH
)

# ── Include paths ──
target_include_directories(appTalkScribe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services
    ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers
)

# miniaudio.h is in src/ — make it findable via <miniaudio.h>
target_include_directories(appTalkScribe SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ── Link Qt ──
target_link_libraries(appTalkScribe PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Concurrent
)

# ── macOS frameworks ──
if(APPLE)
    target_link_libraries(appTalkScribe PRIVATE
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework AudioUnit"
        "-framework ApplicationServices"
        "-framework AppKit"
    )
    set_target_properties(appTalkScribe PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.talkscribe.app"
        MACOSX_BUNDLE_BUNDLE_NAME "TalkScribe"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    )
endif()

# ----------------------------
# whisper.cpp (local speech-to-text)
# ----------------------------
option(TALKSCRIBE_ENABLE_WHISPER "Enable whisper.cpp for local transcription" ON)

if(TALKSCRIBE_ENABLE_WHISPER)
    message(STATUS "Fetching whisper.cpp for local transcription")

    set(_saved_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    set(WHISPER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(WHISPER_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(WHISPER_BUILD_SERVER OFF CACHE BOOL "" FORCE)
    set(GGML_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GGML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    if(APPLE)
        set(GGML_METAL ON CACHE BOOL "" FORCE)
        set(GGML_ACCELERATE ON CACHE BOOL "" FORCE)
        message(STATUS "  Metal + Accelerate enabled for Apple Silicon")
    endif()

    FetchContent_Declare(whisper
        GIT_REPOSITORY https://github.com/ggerganov/whisper.cpp.git
        GIT_TAG        v1.7.3
    )
    FetchContent_MakeAvailable(whisper)

    set(BUILD_SHARED_LIBS ${_saved_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)

    target_link_libraries(appTalkScribe PRIVATE whisper)
    target_compile_definitions(appTalkScribe PRIVATE TALKLESS_HAS_WHISPER=1)

    if(APPLE)
        target_link_libraries(appTalkScribe PRIVATE
            "-framework Metal"
            "-framework MetalKit"
            "-framework Accelerate"
        )
    endif()

    message(STATUS "whisper.cpp enabled (v1.7.3)")
else()
    target_compile_definitions(appTalkScribe PRIVATE TALKLESS_HAS_WHISPER=0)
endif()

# ----------------------------
# sherpa-onnx (Parakeet TDT)
# ----------------------------
option(TALKSCRIBE_ENABLE_SHERPA_ONNX "Enable sherpa-onnx for Parakeet transcription" ON)

if(TALKSCRIBE_ENABLE_SHERPA_ONNX)
    message(STATUS "Fetching sherpa-onnx for Parakeet transcription...")

    set(SHERPA_ONNX_ENABLE_PORTAUDIO OFF CACHE BOOL "" FORCE)
    set(SHERPA_ONNX_ENABLE_BINARY OFF CACHE BOOL "" FORCE)
    set(SHERPA_ONNX_ENABLE_WEBSOCKET OFF CACHE BOOL "" FORCE)
    set(SHERPA_ONNX_ENABLE_TTS OFF CACHE BOOL "" FORCE)
    set(SHERPA_ONNX_ENABLE_SPEAKER_DIARIZATION OFF CACHE BOOL "" FORCE)
    set(SHERPA_ONNX_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
    set(SHERPA_ONNX_ENABLE_C_API ON CACHE BOOL "" FORCE)
    set(SHERPA_ONNX_ENABLE_CHECK OFF CACHE BOOL "" FORCE)

    set(_saved_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    set(_saved_CXX_STANDARD ${CMAKE_CXX_STANDARD})
    set(CMAKE_CXX_STANDARD 17)

    FetchContent_Declare(sherpa-onnx
        GIT_REPOSITORY https://github.com/k2-fsa/sherpa-onnx.git
        GIT_TAG        v1.12.35
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(sherpa-onnx)

    set(CMAKE_CXX_STANDARD ${_saved_CXX_STANDARD})
    set(BUILD_SHARED_LIBS ${_saved_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)

    target_link_libraries(appTalkScribe PRIVATE sherpa-onnx-c-api)
    target_compile_definitions(appTalkScribe PRIVATE TALKLESS_HAS_SHERPA_ONNX=1)
    target_compile_definitions(appTalkScribe PRIVATE TALKLESS_HAS_ONNX=1)

    message(STATUS "sherpa-onnx enabled (v1.12.35)")
else()
    target_compile_definitions(appTalkScribe PRIVATE TALKLESS_HAS_SHERPA_ONNX=0)
    target_compile_definitions(appTalkScribe PRIVATE TALKLESS_HAS_ONNX=0)
endif()

# ----------------------------
# SpeexDSP (resampling 48kHz → 16kHz)
# ----------------------------
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(SPEEXDSP QUIET speexdsp)
endif()

if(NOT SPEEXDSP_FOUND)
    find_path(SPEEXDSP_INCLUDE_DIR
        NAMES speex/speex_preprocess.h
        HINTS /usr/local/include /opt/homebrew/include
    )
    find_library(SPEEXDSP_LIBRARY
        NAMES speexdsp libspeexdsp
        HINTS /usr/local/lib /opt/homebrew/lib
    )
endif()

if(SPEEXDSP_FOUND OR (SPEEXDSP_INCLUDE_DIR AND SPEEXDSP_LIBRARY))
    target_include_directories(appTalkScribe PRIVATE ${SPEEXDSP_INCLUDE_DIRS} ${SPEEXDSP_INCLUDE_DIR})
    target_link_libraries(appTalkScribe PRIVATE ${SPEEXDSP_LIBRARIES} ${SPEEXDSP_LIBRARY})
    target_compile_definitions(appTalkScribe PRIVATE TALKLESS_HAS_SPEEXDSP=1)
    message(STATUS "SpeexDSP found for high-quality resampling")
else()
    message(WARNING
        "SpeexDSP not found. "
        "Install via: brew install speex (macOS) or vcpkg install speexdsp (Windows)"
    )
    target_compile_definitions(appTalkScribe PRIVATE TALKLESS_HAS_SPEEXDSP=0)
endif()

# ── Install ──
install(TARGETS appTalkScribe
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
