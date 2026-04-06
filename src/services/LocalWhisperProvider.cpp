#include "LocalWhisperProvider.h"
#include <QDebug>
#include <QDateTime>
#include <QRegularExpression>
#include <cstring>

// miniaudio ring buffer (from audioEngine)
#include <miniaudio.h>

LocalWhisperProvider::LocalWhisperProvider(QObject* parent)
    : QObject(parent)
{
    // Pre-allocate sliding window: 4 seconds at 16kHz
    m_windowBuffer.resize(WINDOW_SECONDS * kWhisperSampleRate, 0.0f);

    // Scratch buffers for resampling
    m_resampleScratch.resize(4096);
    m_drainScratch.resize(4096);
}

LocalWhisperProvider::~LocalWhisperProvider()
{
    stop();
    unloadModel();

    // Free ring buffer
    if (m_ringBuffer) {
        auto* ringBuffer = static_cast<ma_pcm_rb*>(m_ringBuffer);
        ma_pcm_rb_uninit(ringBuffer);
        delete ringBuffer;
        m_ringBuffer = nullptr;
    }
    free(m_ringBufferData);
    m_ringBufferData = nullptr;
}

bool LocalWhisperProvider::loadModel(const QString& modelPath)
{
#if defined(TALKLESS_HAS_WHISPER) && TALKLESS_HAS_WHISPER
    {
        std::lock_guard<std::mutex> lock(m_ctxMutex);
        if (m_ctx) {
            whisper_free(m_ctx);
            m_ctx = nullptr;
        }

        whisper_context_params cparams = whisper_context_default_params();
        cparams.use_gpu = true; // Metal on macOS

        m_ctx = whisper_init_from_file_with_params(modelPath.toStdString().c_str(), cparams);
    }
    if (!m_ctx) {
        qWarning() << "LocalWhisperProvider: Failed to load model from" << modelPath;
        m_modelLoaded.store(false, std::memory_order_relaxed);
        emit modelLoadedChanged();
        return false;
    }

    m_modelLoaded.store(true, std::memory_order_relaxed);
    emit modelLoadedChanged();
    qDebug() << "LocalWhisperProvider: Model loaded from" << modelPath;
    return true;
#else
    Q_UNUSED(modelPath)
    qWarning() << "LocalWhisperProvider: whisper.cpp not compiled in (TALKLESS_HAS_WHISPER=0)";
    return false;
#endif
}

void LocalWhisperProvider::unloadModel()
{
#if defined(TALKLESS_HAS_WHISPER) && TALKLESS_HAS_WHISPER
    {
        std::lock_guard<std::mutex> lock(m_ctxMutex);
        if (m_ctx) {
            whisper_free(m_ctx);
            m_ctx = nullptr;
        }
    }
#endif
    m_modelLoaded.store(false, std::memory_order_relaxed);
    emit modelLoadedChanged();
}

void LocalWhisperProvider::start()
{
    if (m_active.load(std::memory_order_relaxed))
        return;

    if (!m_modelLoaded.load(std::memory_order_relaxed)) {
        qWarning() << "LocalWhisperProvider: Cannot start — no model loaded";
        return;
    }

    // Initialize ring buffer
    if (!m_ringBuffer) {
        auto* ringBuffer = new ma_pcm_rb;
        m_ringBuffer = ringBuffer;
        const size_t bufferBytes = RING_BUFFER_FRAMES * sizeof(float);
        m_ringBufferData = malloc(bufferBytes);

        if (ma_pcm_rb_init(ma_format_f32, 1, RING_BUFFER_FRAMES,
                           m_ringBufferData, nullptr, ringBuffer) != MA_SUCCESS) {
            qWarning() << "LocalWhisperProvider: Failed to init ring buffer";
            free(m_ringBufferData);
            m_ringBufferData = nullptr;
            delete ringBuffer;
            m_ringBuffer = nullptr;
            return;
        }
    }

    // Reset sliding window
    m_windowWritePos = 0;
    std::fill(m_windowBuffer.begin(), m_windowBuffer.end(), 0.0f);

    // Initialize resampler: input rate → 16kHz
    m_resampler.init(1, m_inputSampleRate, kWhisperSampleRate, 10);

    m_active.store(true, std::memory_order_relaxed);
    emit isActiveChanged();

    startAccumulatorThread();
    startInferenceThread();

    qDebug() << "LocalWhisperProvider: Started (" << m_inputSampleRate << "Hz →" << kWhisperSampleRate << "Hz)";
}

void LocalWhisperProvider::stop()
{
    if (!m_active.load(std::memory_order_relaxed))
        return;

    m_active.store(false, std::memory_order_relaxed);

    stopInferenceThread();
    stopAccumulatorThread();

    // Reset ring buffer
    if (m_ringBuffer)
        ma_pcm_rb_reset(static_cast<ma_pcm_rb*>(m_ringBuffer));

    emit isActiveChanged();
    qDebug() << "LocalWhisperProvider: Stopped";
}

bool LocalWhisperProvider::flushRemaining()
{
    // Drain ALL remaining audio from ring buffer (loop handles wrap-around)
    if (m_ringBuffer) {
        for (;;) {
            void* pRead = nullptr;
            ma_uint32 available = 16384;
            if (ma_pcm_rb_acquire_read(static_cast<ma_pcm_rb*>(m_ringBuffer), &available, &pRead) != MA_SUCCESS || available == 0)
                break;

            if (m_drainScratch.size() < available)
                m_drainScratch.resize(available);
            std::memcpy(m_drainScratch.data(), pRead, available * sizeof(float));
            ma_pcm_rb_commit_read(static_cast<ma_pcm_rb*>(m_ringBuffer), available);

            uint64_t inLen = available;
            uint64_t outLen = m_resampleScratch.size();
            if (m_resampleScratch.size() < available)
                m_resampleScratch.resize(available);
            m_resampler.process(m_drainScratch.data(), &inLen,
                                m_resampleScratch.data(), &outLen);

            for (uint64_t i = 0; i < outLen; ++i) {
                m_windowBuffer[m_windowWritePos % m_windowBuffer.size()] = m_resampleScratch[i];
                m_windowWritePos++;
            }
        }
    }

    // If there's any audio in the window buffer, run a final inference
    const size_t minSamples = kWhisperSampleRate / 2; // at least 0.5s
    if (m_windowWritePos < minSamples) {
        qDebug() << "LocalWhisperProvider: flushRemaining — not enough audio:" << m_windowWritePos << "samples";
        return false;
    }

    const size_t totalSamples = qMin(m_windowWritePos, m_windowBuffer.size());
    std::vector<float> finalChunk(totalSamples);
    const size_t windowSize = m_windowBuffer.size();
    const size_t startPos = (m_windowWritePos > windowSize) ? (m_windowWritePos - windowSize) : 0;
    for (size_t i = 0; i < totalSamples; ++i) {
        finalChunk[i] = m_windowBuffer[(startPos + i) % windowSize];
    }

    qDebug() << "LocalWhisperProvider: flushRemaining — running final inference on"
             << totalSamples << "samples (" << (totalSamples / kWhisperSampleRate) << "s)";

    runInference(std::move(finalChunk));
    return true;
}

void LocalWhisperProvider::feedAudio(const float* samples, uint32_t frameCount)
{
    // Called from audio callback thread — MUST be lock-free
    if (!m_ringBuffer || !m_active.load(std::memory_order_relaxed))
        return;

    void* pWrite = nullptr;
    ma_uint32 framesToWrite = frameCount;

    if (ma_pcm_rb_acquire_write(static_cast<ma_pcm_rb*>(m_ringBuffer), &framesToWrite, &pWrite) == MA_SUCCESS) {
        if (framesToWrite > 0) {
            std::memcpy(pWrite, samples, framesToWrite * sizeof(float));
            ma_pcm_rb_commit_write(static_cast<ma_pcm_rb*>(m_ringBuffer), framesToWrite);
        }
    }
}

void LocalWhisperProvider::setInputSampleRate(uint32_t rate)
{
    m_inputSampleRate = rate;
}

void LocalWhisperProvider::setLanguage(const QString& code)
{
    m_language = code;
}

// ── Accumulator thread ──

void LocalWhisperProvider::startAccumulatorThread()
{
    m_accumulatorThread = new QThread;
    m_accumulatorTimer = new QTimer;
    m_accumulatorTimer->setInterval(100); // 100ms tick
    m_accumulatorTimer->moveToThread(m_accumulatorThread);

    connect(m_accumulatorThread, &QThread::started, m_accumulatorTimer, [this]() {
        m_accumulatorTimer->start();
    });
    connect(m_accumulatorTimer, &QTimer::timeout, this, &LocalWhisperProvider::onAccumulatorTick,
            Qt::DirectConnection);
    connect(m_accumulatorThread, &QThread::finished, m_accumulatorTimer, &QTimer::deleteLater);
    connect(m_accumulatorThread, &QThread::finished, m_accumulatorThread, &QThread::deleteLater);

    m_accumulatorThread->start();
}

void LocalWhisperProvider::stopAccumulatorThread()
{
    if (m_accumulatorThread) {
        m_accumulatorThread->quit();
        m_accumulatorThread->wait(2000);
        m_accumulatorTimer = nullptr;
        m_accumulatorThread = nullptr;
    }
}

void LocalWhisperProvider::onAccumulatorTick()
{
    if (!m_ringBuffer || !m_active.load(std::memory_order_relaxed))
        return;

    // Drain ring buffer
    void* pRead = nullptr;
    ma_uint32 available = 4096;
    if (ma_pcm_rb_acquire_read(static_cast<ma_pcm_rb*>(m_ringBuffer), &available, &pRead) != MA_SUCCESS || available == 0)
        return;

    // Copy to drain scratch
    if (m_drainScratch.size() < available)
        m_drainScratch.resize(available);
    std::memcpy(m_drainScratch.data(), pRead, available * sizeof(float));
    ma_pcm_rb_commit_read(static_cast<ma_pcm_rb*>(m_ringBuffer), available);

    // Resample: inputRate → 16kHz
    uint64_t inLen = available;
    uint64_t outLen = m_resampleScratch.size();
    m_resampler.process(m_drainScratch.data(), &inLen,
                        m_resampleScratch.data(), &outLen);

    // Append to sliding window
    for (uint64_t i = 0; i < outLen; ++i) {
        m_windowBuffer[m_windowWritePos % m_windowBuffer.size()] = m_resampleScratch[i];
        m_windowWritePos++;
    }

    // Check if we have enough for a chunk (3 seconds at 16kHz)
    const size_t chunkSamples = CHUNK_SECONDS * kWhisperSampleRate;
    if (m_windowWritePos >= chunkSamples && !m_chunkReady.load(std::memory_order_relaxed)) {
        // Copy chunk for inference
        std::vector<float> chunk(chunkSamples);
        const size_t windowSize = m_windowBuffer.size();
        const size_t startPos = (m_windowWritePos - chunkSamples) % windowSize;
        for (size_t i = 0; i < chunkSamples; ++i) {
            chunk[i] = m_windowBuffer[(startPos + i) % windowSize];
        }

        // Slide window: keep overlap
        const size_t overlapSamples = OVERLAP_SECONDS * kWhisperSampleRate;
        m_windowWritePos = overlapSamples;
        const size_t copyStart = chunkSamples - overlapSamples;
        for (size_t i = 0; i < overlapSamples; ++i) {
            m_windowBuffer[i] = chunk[copyStart + i];
        }

        {
            std::lock_guard<std::mutex> lock(m_chunkMutex);
            m_chunkStaging = std::move(chunk);
        }
        m_chunkReady.store(true, std::memory_order_release);
    }
}

// ── Inference thread ──

void LocalWhisperProvider::startInferenceThread()
{
    m_inferenceRunning.store(true, std::memory_order_relaxed);
    m_inferenceThread = std::thread([this]() {
        while (m_inferenceRunning.load(std::memory_order_relaxed)) {
            if (m_chunkReady.load(std::memory_order_acquire)) {
                {
                    std::lock_guard<std::mutex> lock(m_chunkMutex);
                    m_chunkActive.swap(m_chunkStaging);
                }
                m_chunkReady.store(false, std::memory_order_release);
                runInference(std::move(m_chunkActive));
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    });
}

void LocalWhisperProvider::stopInferenceThread()
{
    m_inferenceRunning.store(false, std::memory_order_relaxed);
    if (m_inferenceThread.joinable())
        m_inferenceThread.join();
}

void LocalWhisperProvider::runInference(std::vector<float> chunk)
{
#if defined(TALKLESS_HAS_WHISPER) && TALKLESS_HAS_WHISPER
    std::lock_guard<std::mutex> lock(m_ctxMutex);
    if (!m_ctx) return;

    const auto t0 = std::chrono::steady_clock::now();

    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.print_progress = false;
    params.print_special = false;
    params.print_realtime = false;
    params.print_timestamps = false;
    params.translate = false;
    params.no_context = false;
    params.single_segment = true;
    const std::string langStr = m_language.toStdString();
    params.language = m_language == QStringLiteral("auto") ? nullptr : langStr.c_str();
    params.n_threads = m_threadCount;
    params.suppress_non_speech_tokens = true;

    if (whisper_full(m_ctx, params, chunk.data(), static_cast<int>(chunk.size())) != 0) {
        qWarning() << "LocalWhisperProvider: whisper_full failed";
        return;
    }

    const auto t1 = std::chrono::steady_clock::now();
    const int elapsedMs = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());

    const int nSegments = whisper_full_n_segments(m_ctx);
    QString result;
    for (int i = 0; i < nSegments; ++i) {
        const char* text = whisper_full_get_segment_text(m_ctx, i);
        if (text)
            result += QString::fromUtf8(text);
    }

    result = result.trimmed();

    // Strip non-speech annotations Whisper sometimes hallucinates
    // (e.g. "* Musik *", "[Music]", "(Applause)", "♪", silence dots "...")
    static const QRegularExpression nonSpeechRe(
        QStringLiteral(R"(\*[^*]+\*|\[[^\]]+\]|\([^)]+\)|[♪♫🎵🎶]|\.{2,})"));
    result.replace(nonSpeechRe, QString());
    // Collapse multiple spaces left behind by removals
    static const QRegularExpression multiSpaceRe(QStringLiteral(R"(\s{2,})"));
    result.replace(multiSpaceRe, QStringLiteral(" "));
    result = result.trimmed();

    const qint64 ts = QDateTime::currentMSecsSinceEpoch();

    // Deliver result + timing to main thread
    QMetaObject::invokeMethod(this, [this, result, ts, elapsedMs]() {
        emit inferenceTimeMs(elapsedMs);
        if (!result.isEmpty())
            emit textReady(result, ts);
    }, Qt::QueuedConnection);
#else
    Q_UNUSED(chunk)
#endif
}
