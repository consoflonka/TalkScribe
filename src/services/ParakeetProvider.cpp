#include "ParakeetProvider.h"
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <chrono>
#include <cstring>
#include <miniaudio.h>

#if defined(TALKLESS_HAS_SHERPA_ONNX) && TALKLESS_HAS_SHERPA_ONNX
#include "sherpa-onnx/c-api/c-api.h"
#endif

ParakeetProvider::ParakeetProvider(QObject* parent)
    : QObject(parent)
{
    m_resampleScratch.resize(4096);
    m_drainScratch.resize(4096);
}

ParakeetProvider::~ParakeetProvider()
{
    stop();
    unloadModel();

    if (m_ringBuffer) {
        auto* ringBuffer = static_cast<ma_pcm_rb*>(m_ringBuffer);
        ma_pcm_rb_uninit(ringBuffer);
        delete ringBuffer;
        m_ringBuffer = nullptr;
    }
    free(m_ringBufferData);
    m_ringBufferData = nullptr;
}

bool ParakeetProvider::isAvailable()
{
#if defined(TALKLESS_HAS_SHERPA_ONNX) && TALKLESS_HAS_SHERPA_ONNX
    return true;
#else
    return false;
#endif
}

bool ParakeetProvider::loadModel(const QString& modelPath)
{
#if defined(TALKLESS_HAS_SHERPA_ONNX) && TALKLESS_HAS_SHERPA_ONNX
    std::lock_guard<std::mutex> lock(m_ctxMutex);

    // Clean up previous handles (inline — can't call unloadModel() as it also locks m_ctxMutex)
    if (m_recognizer) {
        SherpaOnnxDestroyOfflineRecognizer(m_recognizer);
        m_recognizer = nullptr;
    }
    if (m_vad) {
        SherpaOnnxDestroyVoiceActivityDetector(m_vad);
        m_vad = nullptr;
    }
    m_modelLoaded.store(false, std::memory_order_relaxed);

    const QDir modelDir(modelPath);

    // ── Silero VAD ──
    // Look for silero_vad.onnx in model dir, then in app data
    QString vadModelPath = modelDir.filePath(QStringLiteral("silero_vad.onnx"));
    if (!QFileInfo::exists(vadModelPath)) {
        // Fallback: app data directory
        vadModelPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                       + QStringLiteral("/whisper-models/silero_vad.onnx");
    }
    if (!QFileInfo::exists(vadModelPath)) {
        qWarning() << "ParakeetProvider: silero_vad.onnx not found — VAD unavailable";
        // Continue without VAD — will use fixed chunks as fallback
    }

    SherpaOnnxVadModelConfig vadCfg;
    memset(&vadCfg, 0, sizeof(vadCfg));

    const QByteArray vadPathUtf8 = vadModelPath.toUtf8();
    vadCfg.silero_vad.model = vadPathUtf8.constData();
    vadCfg.silero_vad.threshold = 0.5f;
    vadCfg.silero_vad.min_silence_duration = 0.5f;
    vadCfg.silero_vad.min_speech_duration = 0.25f;
    vadCfg.silero_vad.window_size = 512;
    vadCfg.sample_rate = TARGET_SAMPLE_RATE;
    vadCfg.num_threads = 1;
    vadCfg.provider = "cpu";
    vadCfg.debug = 0;

    if (QFileInfo::exists(vadModelPath)) {
        qDebug() << "ParakeetProvider: Creating Silero VAD from" << vadModelPath << "...";
        m_vad = SherpaOnnxCreateVoiceActivityDetector(&vadCfg, 30.0f);
        qDebug() << "ParakeetProvider: VAD creation returned:" << (m_vad ? "OK" : "NULL");
    } else {
        qDebug() << "ParakeetProvider: No VAD model found, using fallback chunking";
    }

    // ── Parakeet TDT Offline Recognizer ──
    SherpaOnnxOfflineRecognizerConfig cfg;
    memset(&cfg, 0, sizeof(cfg));

    const QByteArray encoderPath = modelDir.filePath(QStringLiteral("encoder.int8.onnx")).toUtf8();
    const QByteArray decoderPath = modelDir.filePath(QStringLiteral("decoder.int8.onnx")).toUtf8();
    const QByteArray joinerPath = modelDir.filePath(QStringLiteral("joiner.int8.onnx")).toUtf8();
    const QByteArray tokensPath = modelDir.filePath(QStringLiteral("tokens.txt")).toUtf8();

    cfg.model_config.transducer.encoder = encoderPath.constData();
    cfg.model_config.transducer.decoder = decoderPath.constData();
    cfg.model_config.transducer.joiner = joinerPath.constData();
    cfg.model_config.tokens = tokensPath.constData();
    cfg.model_config.model_type = "nemo_transducer";
    cfg.model_config.provider = "cpu";
    cfg.model_config.num_threads = m_threadCount;
    cfg.model_config.debug = 0;
    cfg.decoding_method = "greedy_search";
    cfg.feat_config.sample_rate = TARGET_SAMPLE_RATE;
    cfg.feat_config.feature_dim = 80;

    qDebug() << "ParakeetProvider: Creating offline recognizer (this may take a moment)..."
             << "encoder:" << encoderPath
             << "decoder:" << decoderPath
             << "joiner:" << joinerPath
             << "tokens:" << tokensPath;
    m_recognizer = SherpaOnnxCreateOfflineRecognizer(&cfg);
    qDebug() << "ParakeetProvider: CreateOfflineRecognizer returned:" << (m_recognizer ? "OK" : "NULL");
    if (!m_recognizer) {
        qWarning() << "ParakeetProvider: Failed to create recognizer from" << modelPath;
        if (m_vad) { SherpaOnnxDestroyVoiceActivityDetector(m_vad); m_vad = nullptr; }
        return false;
    }

    m_modelLoaded.store(true, std::memory_order_relaxed);
    emit modelLoadedChanged();
    qDebug() << "ParakeetProvider: Model loaded from" << modelPath;
    return true;
#else
    Q_UNUSED(modelPath)
    qWarning() << "ParakeetProvider: sherpa-onnx not compiled in";
    return false;
#endif
}

void ParakeetProvider::unloadModel()
{
#if defined(TALKLESS_HAS_SHERPA_ONNX) && TALKLESS_HAS_SHERPA_ONNX
    {
        std::lock_guard<std::mutex> lock(m_ctxMutex);
        if (m_recognizer) {
            SherpaOnnxDestroyOfflineRecognizer(m_recognizer);
            m_recognizer = nullptr;
        }
        if (m_vad) {
            SherpaOnnxDestroyVoiceActivityDetector(m_vad);
            m_vad = nullptr;
        }
    }
#endif
    m_modelLoaded.store(false, std::memory_order_relaxed);
    emit modelLoadedChanged();
}

void ParakeetProvider::start()
{
    if (m_active.load(std::memory_order_relaxed)) return;
    if (!m_modelLoaded.load(std::memory_order_relaxed)) {
        qWarning() << "ParakeetProvider: Cannot start — no model loaded";
        return;
    }

    if (!m_ringBuffer) {
        auto* ringBuffer = new ma_pcm_rb;
        m_ringBuffer = ringBuffer;
        const size_t bufferBytes = RING_BUFFER_FRAMES * sizeof(float);
        m_ringBufferData = malloc(bufferBytes);
        if (ma_pcm_rb_init(ma_format_f32, 1, RING_BUFFER_FRAMES,
                           m_ringBufferData, nullptr, ringBuffer) != MA_SUCCESS) {
            qWarning() << "ParakeetProvider: Failed to init ring buffer";
            free(m_ringBufferData); m_ringBufferData = nullptr;
            delete ringBuffer; m_ringBuffer = nullptr;
            return;
        }
    }

    m_resampler.init(1, m_inputSampleRate, TARGET_SAMPLE_RATE, 10);
    m_audioAccumulator.clear();

    m_active.store(true, std::memory_order_relaxed);
    emit isActiveChanged();

    startAccumulatorThread();
    startInferenceThread();
    qDebug() << "ParakeetProvider: Started";
}

void ParakeetProvider::stop()
{
    if (!m_active.load(std::memory_order_relaxed)) return;
    m_active.store(false, std::memory_order_relaxed);
    stopInferenceThread();
    stopAccumulatorThread();
    if (m_ringBuffer) ma_pcm_rb_reset(static_cast<ma_pcm_rb*>(m_ringBuffer));

#if defined(TALKLESS_HAS_SHERPA_ONNX) && TALKLESS_HAS_SHERPA_ONNX
    // Reset VAD state for next session
    if (m_vad) SherpaOnnxVoiceActivityDetectorReset(m_vad);
#endif

    emit isActiveChanged();
}

bool ParakeetProvider::flushRemaining()
{
#if defined(TALKLESS_HAS_SHERPA_ONNX) && TALKLESS_HAS_SHERPA_ONNX
    // 0. Stop accumulator + inference threads first to avoid race conditions.
    //    They might still be draining the ring buffer or processing chunks.
    stopInferenceThread();
    stopAccumulatorThread();

    // 1. Drain ALL remaining audio from ring buffer (loop handles wrap-around)
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

            if (outLen > 0)
                m_audioAccumulator.insert(m_audioAccumulator.end(),
                                          m_resampleScratch.begin(),
                                          m_resampleScratch.begin() + static_cast<ptrdiff_t>(outLen));
        }
    }

    // 2. Also grab any pending staging chunks
    {
        std::lock_guard<std::mutex> lock(m_chunkMutex);
        if (!m_chunkStaging.empty()) {
            m_audioAccumulator.insert(m_audioAccumulator.end(),
                                      m_chunkStaging.begin(), m_chunkStaging.end());
            m_chunkStaging.clear();
        }
    }

    std::lock_guard<std::mutex> lock(m_ctxMutex);
    if (!m_recognizer) return false;

    // 3. Feed any remaining audio to VAD
    if (m_vad && !m_audioAccumulator.empty()) {
        qDebug() << "ParakeetProvider: flushRemaining — feeding" << m_audioAccumulator.size()
                 << "samples to VAD";
        SherpaOnnxVoiceActivityDetectorAcceptWaveform(
            m_vad, m_audioAccumulator.data(), static_cast<int32_t>(m_audioAccumulator.size()));
        m_audioAccumulator.clear();
    } else if (!m_vad && !m_audioAccumulator.empty()) {
        // No VAD — process entire accumulated audio as one chunk
        const SherpaOnnxOfflineStream* stream = SherpaOnnxCreateOfflineStream(m_recognizer);
        SherpaOnnxAcceptWaveformOffline(stream, TARGET_SAMPLE_RATE,
                                        m_audioAccumulator.data(),
                                        static_cast<int32_t>(m_audioAccumulator.size()));
        SherpaOnnxDecodeOfflineStream(m_recognizer, stream);

        const SherpaOnnxOfflineRecognizerResult* result =
            SherpaOnnxGetOfflineStreamResult(stream);
        if (result && result->text && strlen(result->text) > 0) {
            const QString text = QString::fromUtf8(result->text).trimmed();
            const qint64 ts = QDateTime::currentMSecsSinceEpoch();
            QMetaObject::invokeMethod(this, [this, text, ts]() {
                if (!text.isEmpty())
                    emit textReady(text, ts);
            }, Qt::QueuedConnection);
        }
        SherpaOnnxDestroyOfflineRecognizerResult(result);
        SherpaOnnxDestroyOfflineStream(stream);
        m_audioAccumulator.clear();
        return true;
    }

    // 4. ALWAYS flush VAD — even if no new audio. The inference thread may
    //    have already fed audio to the VAD but it hasn't emitted the last
    //    segment yet (because it hasn't seen enough silence).
    if (m_vad) {
        // Add 500ms silence padding to help VAD detect end-of-speech
        std::vector<float> silencePadding(TARGET_SAMPLE_RATE / 2, 0.0f);
        SherpaOnnxVoiceActivityDetectorAcceptWaveform(
            m_vad, silencePadding.data(), static_cast<int32_t>(silencePadding.size()));
        SherpaOnnxVoiceActivityDetectorFlush(m_vad);

        bool hadSegments = false;
        while (!SherpaOnnxVoiceActivityDetectorEmpty(m_vad)) {
            const SherpaOnnxSpeechSegment* segment = SherpaOnnxVoiceActivityDetectorFront(m_vad);
            if (segment && segment->n > 0) {
                hadSegments = true;
                const SherpaOnnxOfflineStream* stream = SherpaOnnxCreateOfflineStream(m_recognizer);
                SherpaOnnxAcceptWaveformOffline(stream, TARGET_SAMPLE_RATE,
                                                segment->samples, segment->n);
                SherpaOnnxDecodeOfflineStream(m_recognizer, stream);

                const SherpaOnnxOfflineRecognizerResult* result =
                    SherpaOnnxGetOfflineStreamResult(stream);
                if (result && result->text && strlen(result->text) > 0) {
                    const QString text = QString::fromUtf8(result->text).trimmed();
                    const qint64 ts = QDateTime::currentMSecsSinceEpoch();

                    qDebug() << "ParakeetProvider: flushRemaining text:" << text;
                    QMetaObject::invokeMethod(this, [this, text, ts]() {
                        if (!text.isEmpty())
                            emit textReady(text, ts);
                    }, Qt::QueuedConnection);
                }
                SherpaOnnxDestroyOfflineRecognizerResult(result);
                SherpaOnnxDestroyOfflineStream(stream);
            }
            SherpaOnnxDestroySpeechSegment(segment);
            SherpaOnnxVoiceActivityDetectorPop(m_vad);
        }

        qDebug() << "ParakeetProvider: flushRemaining complete, hadSegments:" << hadSegments;
        return hadSegments;
    }
#endif
    return false;
}

void ParakeetProvider::feedAudio(const float* samples, uint32_t frameCount)
{
    if (!m_ringBuffer || !m_active.load(std::memory_order_relaxed)) return;
    void* pWrite = nullptr;
    ma_uint32 framesToWrite = frameCount;
    if (ma_pcm_rb_acquire_write(static_cast<ma_pcm_rb*>(m_ringBuffer), &framesToWrite, &pWrite) == MA_SUCCESS && framesToWrite > 0) {
        std::memcpy(pWrite, samples, framesToWrite * sizeof(float));
        ma_pcm_rb_commit_write(static_cast<ma_pcm_rb*>(m_ringBuffer), framesToWrite);
    }
}

void ParakeetProvider::setInputSampleRate(uint32_t rate) { m_inputSampleRate = rate; }
void ParakeetProvider::setLanguage(const QString& code) { m_language = code; }

// ── Accumulator thread ──

void ParakeetProvider::startAccumulatorThread()
{
    m_accumulatorThread = new QThread;
    m_accumulatorTimer = new QTimer;
    m_accumulatorTimer->setInterval(100);
    m_accumulatorTimer->moveToThread(m_accumulatorThread);
    connect(m_accumulatorThread, &QThread::started, m_accumulatorTimer, [this]() { m_accumulatorTimer->start(); });
    connect(m_accumulatorTimer, &QTimer::timeout, this, &ParakeetProvider::onAccumulatorTick, Qt::DirectConnection);
    connect(m_accumulatorThread, &QThread::finished, m_accumulatorTimer, &QTimer::deleteLater);
    connect(m_accumulatorThread, &QThread::finished, m_accumulatorThread, &QThread::deleteLater);
    m_accumulatorThread->start();
}

void ParakeetProvider::stopAccumulatorThread()
{
    if (m_accumulatorThread) {
        m_accumulatorThread->quit();
        m_accumulatorThread->wait(2000);
        m_accumulatorTimer = nullptr;
        m_accumulatorThread = nullptr;
    }
}

void ParakeetProvider::onAccumulatorTick()
{
    if (!m_ringBuffer || !m_active.load(std::memory_order_relaxed)) return;

    // Drain ring buffer
    void* pRead = nullptr;
    ma_uint32 available = 4096;
    if (ma_pcm_rb_acquire_read(static_cast<ma_pcm_rb*>(m_ringBuffer), &available, &pRead) != MA_SUCCESS || available == 0) return;

    if (m_drainScratch.size() < available) m_drainScratch.resize(available);
    std::memcpy(m_drainScratch.data(), pRead, available * sizeof(float));
    ma_pcm_rb_commit_read(static_cast<ma_pcm_rb*>(m_ringBuffer), available);

    // Resample to 16kHz
    uint64_t inLen = available;
    uint64_t outLen = m_resampleScratch.size();
    m_resampler.process(m_drainScratch.data(), &inLen, m_resampleScratch.data(), &outLen);

    if (outLen == 0) return;

    // Pass resampled audio to inference thread via staging buffer
    if (!m_chunkReady.load(std::memory_order_relaxed)) {
        std::lock_guard<std::mutex> lock(m_chunkMutex);
        m_chunkStaging.insert(m_chunkStaging.end(),
                              m_resampleScratch.begin(),
                              m_resampleScratch.begin() + static_cast<ptrdiff_t>(outLen));

        // Send to inference every ~500ms of accumulated audio (8000 samples at 16kHz)
        if (m_chunkStaging.size() >= 8000) {
            m_chunkReady.store(true, std::memory_order_release);
        }
    }
}

// ── Inference thread ──

void ParakeetProvider::startInferenceThread()
{
    m_inferenceRunning.store(true, std::memory_order_relaxed);
    m_inferenceThread = std::thread([this]() {
        while (m_inferenceRunning.load(std::memory_order_relaxed)) {
            if (m_chunkReady.load(std::memory_order_acquire)) {
                std::vector<float> chunk;
                {
                    std::lock_guard<std::mutex> lock(m_chunkMutex);
                    chunk.swap(m_chunkStaging);
                }
                m_chunkReady.store(false, std::memory_order_release);
                processAudioChunk(std::move(chunk));
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    });
}

void ParakeetProvider::stopInferenceThread()
{
    m_inferenceRunning.store(false, std::memory_order_relaxed);
    if (m_inferenceThread.joinable()) m_inferenceThread.join();
}

void ParakeetProvider::processAudioChunk(std::vector<float> chunk)
{
#if defined(TALKLESS_HAS_SHERPA_ONNX) && TALKLESS_HAS_SHERPA_ONNX
    std::lock_guard<std::mutex> lock(m_ctxMutex);
    if (!m_recognizer) return;

    const auto t0 = std::chrono::steady_clock::now();

    if (m_vad) {
        // ── VAD-based segmentation ──
        SherpaOnnxVoiceActivityDetectorAcceptWaveform(
            m_vad, chunk.data(), static_cast<int32_t>(chunk.size()));

        while (!SherpaOnnxVoiceActivityDetectorEmpty(m_vad)) {
            const SherpaOnnxSpeechSegment* segment = SherpaOnnxVoiceActivityDetectorFront(m_vad);
            if (segment && segment->n > 0) {
                const SherpaOnnxOfflineStream* stream = SherpaOnnxCreateOfflineStream(m_recognizer);
                SherpaOnnxAcceptWaveformOffline(stream, TARGET_SAMPLE_RATE,
                                                segment->samples, segment->n);
                SherpaOnnxDecodeOfflineStream(m_recognizer, stream);

                const SherpaOnnxOfflineRecognizerResult* result =
                    SherpaOnnxGetOfflineStreamResult(stream);
                if (result && result->text && strlen(result->text) > 0) {
                    const QString text = QString::fromUtf8(result->text).trimmed();
                    const qint64 ts = QDateTime::currentMSecsSinceEpoch();
                    const auto t1 = std::chrono::steady_clock::now();
                    const int elapsedMs = static_cast<int>(
                        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());

                    QMetaObject::invokeMethod(this, [this, text, ts, elapsedMs]() {
                        emit inferenceTimeMs(elapsedMs);
                        if (!text.isEmpty())
                            emit textReady(text, ts);
                    }, Qt::QueuedConnection);
                }

                SherpaOnnxDestroyOfflineRecognizerResult(result);
                SherpaOnnxDestroyOfflineStream(stream);
            }
            SherpaOnnxDestroySpeechSegment(segment);
            SherpaOnnxVoiceActivityDetectorPop(m_vad);
        }
    } else {
        // ── Fallback: no VAD, process entire chunk as one segment ──
        const SherpaOnnxOfflineStream* stream = SherpaOnnxCreateOfflineStream(m_recognizer);
        SherpaOnnxAcceptWaveformOffline(stream, TARGET_SAMPLE_RATE,
                                        chunk.data(), static_cast<int32_t>(chunk.size()));
        SherpaOnnxDecodeOfflineStream(m_recognizer, stream);

        const SherpaOnnxOfflineRecognizerResult* result =
            SherpaOnnxGetOfflineStreamResult(stream);
        if (result && result->text && strlen(result->text) > 0) {
            const QString text = QString::fromUtf8(result->text).trimmed();
            const qint64 ts = QDateTime::currentMSecsSinceEpoch();
            const auto t1 = std::chrono::steady_clock::now();
            const int elapsedMs = static_cast<int>(
                std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());

            QMetaObject::invokeMethod(this, [this, text, ts, elapsedMs]() {
                emit inferenceTimeMs(elapsedMs);
                if (!text.isEmpty())
                    emit textReady(text, ts);
            }, Qt::QueuedConnection);
        }

        SherpaOnnxDestroyOfflineRecognizerResult(result);
        SherpaOnnxDestroyOfflineStream(stream);
    }
#else
    Q_UNUSED(chunk)
#endif
}
