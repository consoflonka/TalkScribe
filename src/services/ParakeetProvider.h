#pragma once

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QString>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include "../speexResampler.h"

// Forward declarations for sherpa-onnx C API handles
#if defined(TALKLESS_HAS_SHERPA_ONNX) && TALKLESS_HAS_SHERPA_ONNX
struct SherpaOnnxVoiceActivityDetector;
struct SherpaOnnxOfflineRecognizer;
#endif

/**
 * @brief NVIDIA Parakeet speech-to-text via sherpa-onnx.
 *
 * Uses sherpa-onnx (which embeds ONNX Runtime) for Parakeet TDT models.
 * VAD (Silero) segments audio into speech chunks, then offline Parakeet
 * recognizer transcribes each segment.
 *
 * Same public API as LocalWhisperProvider — drop-in replacement.
 */
class ParakeetProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool modelLoaded READ modelLoaded NOTIFY modelLoadedChanged)

public:
    explicit ParakeetProvider(QObject* parent = nullptr);
    ~ParakeetProvider() override;

    [[nodiscard]] bool isActive() const { return m_active.load(std::memory_order_relaxed); }
    [[nodiscard]] bool modelLoaded() const { return m_modelLoaded.load(std::memory_order_relaxed); }

    /// Load Parakeet model from directory path (containing encoder/decoder/joiner/tokens)
    bool loadModel(const QString& modelPath);
    void unloadModel();

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    /// Flush remaining audio: drain ring buffer, flush VAD, process final segments.
    /// Call BEFORE stop() to capture the last words.
    bool flushRemaining();

    /// Feed audio from audio callback (MUST be lock-free)
    void feedAudio(const float* samples, uint32_t frameCount);

    void setInputSampleRate(uint32_t rate);
    void setLanguage(const QString& code);
    void setThreadCount(int count) { m_threadCount = count; }

    /// Check if sherpa-onnx is available at compile time
    static bool isAvailable();

signals:
    void isActiveChanged();
    void modelLoadedChanged();
    void textReady(const QString& text, qint64 timestampMs);
    void inferenceTimeMs(int durationMs);

private:
    void startAccumulatorThread();
    void stopAccumulatorThread();
    void startInferenceThread();
    void stopInferenceThread();
    void onAccumulatorTick();
    void processAudioChunk(std::vector<float> chunk);

    // Ring buffer (lock-free, audio thread → accumulator)
    void* m_ringBuffer = nullptr;
    void* m_ringBufferData = nullptr;
    static constexpr uint32_t RING_BUFFER_FRAMES = 96000;

    // Resampler: input → 16kHz
    SpeexResampler m_resampler;
    uint32_t m_inputSampleRate = 48000;
    static constexpr uint32_t TARGET_SAMPLE_RATE = 16000;

    // Accumulator collects resampled audio and passes to inference
    QThread* m_accumulatorThread = nullptr;
    QTimer* m_accumulatorTimer = nullptr;
    std::vector<float> m_resampleScratch;
    std::vector<float> m_drainScratch;

    // Accumulated audio buffer (VAD processes this)
    std::vector<float> m_audioAccumulator;
    std::mutex m_accumulatorMutex;

    // Inference thread
    std::thread m_inferenceThread;
    std::atomic<bool> m_inferenceRunning{false};
    std::vector<float> m_chunkStaging;
    std::vector<float> m_chunkActive;
    std::mutex m_chunkMutex;
    std::atomic<bool> m_chunkReady{false};

    // sherpa-onnx handles (protected by m_ctxMutex)
    std::mutex m_ctxMutex;
#if defined(TALKLESS_HAS_SHERPA_ONNX) && TALKLESS_HAS_SHERPA_ONNX
    const SherpaOnnxVoiceActivityDetector* m_vad = nullptr;
    const SherpaOnnxOfflineRecognizer* m_recognizer = nullptr;
#else
    void* m_vad = nullptr;
    void* m_recognizer = nullptr;
#endif

    QString m_language = QStringLiteral("de");
    int m_threadCount = 4;
    std::atomic<bool> m_active{false};
    std::atomic<bool> m_modelLoaded{false};
};
