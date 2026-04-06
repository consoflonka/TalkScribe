#pragma once

#include <QObject>
#include <QThread>
#include <QTimer>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <functional>

#if defined(TALKLESS_HAS_WHISPER) && TALKLESS_HAS_WHISPER
#include <whisper.h>
#endif

#include "../speexResampler.h"

/**
 * @brief whisper.cpp wrapper for real-time local speech-to-text.
 *
 * Receives audio from AudioEngine callbacks (lock-free ring buffer write),
 * resamples 48kHz→16kHz, accumulates chunks, and runs inference on a
 * dedicated thread. Emits transcription results back to main thread.
 *
 * Thread architecture:
 *   Audio callback (real-time) → feedAudio() → lock-free ring buffer
 *   Accumulator thread (100ms) → drain + resample → sliding window
 *   Inference thread (std::thread) → whisper_full() → signal
 */
class LocalWhisperProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool modelLoaded READ modelLoaded NOTIFY modelLoadedChanged)

public:
    explicit LocalWhisperProvider(QObject* parent = nullptr);
    ~LocalWhisperProvider() override;

    [[nodiscard]] bool isActive() const { return m_active.load(std::memory_order_relaxed); }
    [[nodiscard]] bool modelLoaded() const { return m_modelLoaded.load(std::memory_order_relaxed); }

    /// Load a whisper model from file path
    bool loadModel(const QString& modelPath);

    /// Unload the current model and free resources
    void unloadModel();

    /// Start transcription processing
    Q_INVOKABLE void start();

    /// Stop transcription processing
    Q_INVOKABLE void stop();

    /// Flush remaining audio in the window buffer as a final inference chunk.
    /// Call BEFORE stop() to ensure no audio is lost.
    /// Returns true if a final chunk was queued for inference.
    bool flushRemaining();

    /// Feed audio from the audio callback thread (MUST be lock-free)
    /// @param samples Mono float32 PCM at input sample rate (typically 48kHz)
    /// @param frameCount Number of frames
    void feedAudio(const float* samples, uint32_t frameCount);

    /// Set source sample rate (default 48000)
    void setInputSampleRate(uint32_t rate);
    void setLanguage(const QString& code);
    void setThreadCount(int count) { m_threadCount = count; }

signals:
    void isActiveChanged();
    void modelLoadedChanged();

    /// Emitted when a chunk has been transcribed (on main thread)
    void textReady(const QString& text, qint64 timestampMs);

    /// Emitted after each inference with wall-clock duration (for performance monitoring)
    void inferenceTimeMs(int durationMs);

private:
    void startAccumulatorThread();
    void stopAccumulatorThread();
    void startInferenceThread();
    void stopInferenceThread();

    // Accumulator: drain ring buffer, resample, accumulate
    void onAccumulatorTick();

    // Inference: run whisper_full on chunk
    void runInference(std::vector<float> chunk);

    // Ring buffer (lock-free, audio thread → accumulator thread)
    void* m_ringBuffer = nullptr;     // ma_pcm_rb* (opaque to avoid miniaudio.h in header)
    void* m_ringBufferData = nullptr;
    static constexpr uint32_t RING_BUFFER_FRAMES = 96000; // 2s at 48kHz

    // Resampler: input rate → 16kHz
    SpeexResampler m_resampler;
    uint32_t m_inputSampleRate = 48000;
    static constexpr uint32_t kWhisperSampleRate = 16000;

    // Sliding window at 16kHz
    std::vector<float> m_windowBuffer;
    size_t m_windowWritePos = 0;
    static constexpr size_t WINDOW_SECONDS = 4;
    static constexpr size_t CHUNK_SECONDS = 3;
    static constexpr size_t OVERLAP_SECONDS = 1;

    // Accumulator thread
    QThread* m_accumulatorThread = nullptr;
    QTimer* m_accumulatorTimer = nullptr;
    std::vector<float> m_resampleScratch;
    std::vector<float> m_drainScratch;

    // Inference thread (double-buffered: accumulator writes to staging, inference reads from active)
    std::thread m_inferenceThread;
    std::atomic<bool> m_inferenceRunning{false};
    std::vector<float> m_chunkStaging;  // Written by accumulator thread
    std::vector<float> m_chunkActive;   // Read by inference thread
    std::mutex m_chunkMutex;            // Protects swap between staging and active
    std::atomic<bool> m_chunkReady{false};

    // Whisper context (protected by m_ctxMutex for safe load/unload during inference)
    std::mutex m_ctxMutex;
#if defined(TALKLESS_HAS_WHISPER) && TALKLESS_HAS_WHISPER
    whisper_context* m_ctx = nullptr;
#else
    void* m_ctx = nullptr;
#endif
    QString m_language = QStringLiteral("de");

    int m_threadCount = 4;
    std::atomic<bool> m_active{false};
    std::atomic<bool> m_modelLoaded{false};
};
