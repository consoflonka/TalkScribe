# TalkScribe

Standalone dictation app — speak, transcribe, paste. Powered by local AI (Whisper.cpp + Parakeet).

## What it does

- Click the floating microphone overlay to start dictating
- Your speech is transcribed locally on your device (no cloud, no API keys)
- Text is automatically copied to clipboard and optionally pasted
- Custom word corrections fix common speech recognition errors
- Snippets expand trigger phrases into full text blocks
- Statistics track your usage over time

## Requirements

- **macOS** (primary platform, Metal GPU acceleration)
- Qt 6.5+
- CMake 3.21+
- SpeexDSP (`brew install speex`)

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="$HOME/Qt/6.8.2/macos"
cmake --build . --parallel
```

First build takes a while as whisper.cpp and sherpa-onnx are fetched and compiled.

## Architecture

Extracted from [TalkLess](https://github.com/consoflonka/TalkLess) — the full dictation + soundboard desktop app. TalkScribe is the isolated dictation component for experimentation.

### Key Components

- **GlobalDictationService** — Main controller (start/stop/toggle, state machine)
- **LocalWhisperProvider** — Whisper.cpp inference with ring buffer + sliding window
- **ParakeetProvider** — Sherpa-ONNX alternative engine with VAD
- **WhisperModelManager** — Download, verify, manage speech models
- **AudioCapture** — Simple miniaudio microphone capture
- **TextPostProcessor** — Custom words (fuzzy matching) + snippets (exact matching)
- **DictationStatsService** — Usage tracking (words, transcriptions, time saved)

## License

Private — for personal experimentation.
