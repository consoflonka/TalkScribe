#include "GlobalDictationService.h"
#include "TextPostProcessor.h"
#include "DictationStatsService.h"
#include "LocalWhisperProvider.h"
#include "ParakeetProvider.h"
#include "WhisperModelManager.h"
#include "../helpers/MacWindowHelper.h"
#include <QDebug>
#include <QDateTime>
#include <QSettings>
#include <cmath>

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

GlobalDictationService::GlobalDictationService(QObject* parent)
    : QObject(parent)
    , m_whisperProvider(std::make_unique<LocalWhisperProvider>())
    , m_parakeetProvider(std::make_unique<ParakeetProvider>())
{
    connect(m_whisperProvider.get(), &LocalWhisperProvider::textReady,
            this, &GlobalDictationService::onWhisperTextReady);
    connect(m_parakeetProvider.get(), &ParakeetProvider::textReady,
            this, &GlobalDictationService::onParakeetTextReady);

    // Silence timeout — auto-stop after N seconds without new text
    m_silenceTimer.setSingleShot(true);
    connect(&m_silenceTimer, &QTimer::timeout, this, [this]() {
        if (m_active.load(std::memory_order_relaxed)) {
            qDebug() << "GlobalDictationService: Silence timeout — auto-stopping";
            stop();
        }
    });

    // Load settings
    QSettings settings;
    m_autoPaste = settings.value(QStringLiteral("dictation/autoPaste"), true).toBool();
    m_overlayVisible = settings.value(QStringLiteral("dictation/overlayVisible"), true).toBool();
    m_silenceTimeoutSecs = settings.value(QStringLiteral("dictation/silenceTimeout"), 30).toInt();
}

GlobalDictationService::~GlobalDictationService()
{
    if (m_active.load(std::memory_order_relaxed))
        stop();
}

void GlobalDictationService::setEnabled(bool enabled)
{
    if (m_enabled == enabled) return;
    m_enabled = enabled;
    emit enabledChanged();
    if (!enabled && m_active.load(std::memory_order_relaxed))
        stop();
}

void GlobalDictationService::toggle()
{
    if (!m_enabled) return; // Not logged in
    if (m_loading) {
        // Cancel loading
        m_loading = false;
        emit isLoadingChanged();
        qDebug() << "GlobalDictationService: Loading cancelled";
        return;
    }
    if (m_stopping.load(std::memory_order_relaxed))
        return; // Already stopping, ignore
    if (m_active.load(std::memory_order_relaxed))
        stop();
    else
        start();
}

void GlobalDictationService::start()
{
    if (m_active.load(std::memory_order_relaxed)) return;
    if (m_loading) return;

    if (!m_modelManager) {
        emit errorOccurred(tr("No model manager available"));
        return;
    }

    // Find a usable model — prefer activeModelId, fallback to any downloaded model
    QString useModelId = m_modelManager->activeModelId();
    QString modelPath = m_modelManager->modelFilePath(useModelId);

    if (modelPath.isEmpty()) {
        // Active model not downloaded — try to find ANY downloaded model
        const auto models = m_modelManager->availableModels();
        for (const auto& mv : models) {
            auto m = mv.toMap();
            if (m[QStringLiteral("downloaded")].toBool() && !m[QStringLiteral("locked")].toBool()) {
                useModelId = m[QStringLiteral("id")].toString();
                modelPath = m_modelManager->modelFilePath(useModelId);
                if (!modelPath.isEmpty()) {
                    m_modelManager->setActiveModel(useModelId);
                    qDebug() << "GlobalDictationService: Fallback to available model:" << useModelId;
                    break;
                }
            }
        }
    }

    if (modelPath.isEmpty()) {
        emit errorOccurred(tr("No model downloaded. Please download a model in Settings → AI & Tools."));
        return;
    }

    // Remember which app was focused so we can return to it after paste
    if (m_macWindowHelper)
        m_macWindowHelper->saveFrontmostApp();

    m_loading = true;
    emit isLoadingChanged();

    // Determine engine type
    const auto models = m_modelManager->availableModels();
    for (const auto& mv : models) {
        auto m = mv.toMap();
        if (m[QStringLiteral("id")].toString() == useModelId) {
            m_useParakeet.store(m[QStringLiteral("engine")].toString() == QStringLiteral("parakeet"),
                                std::memory_order_relaxed);
            break;
        }
    }

    // Clear previous text and reset cancelled flag
    m_currentText.clear();
    m_finalText.clear();
    m_cancelled.store(false, std::memory_order_relaxed);
    emit textUpdated();

    // Start recording timer for stats tracking
    m_recordingTimer.start();

    // Configure language and thread count
    const bool isParakeet = m_useParakeet.load(std::memory_order_relaxed);
    const QString lang = m_modelManager->autoDetectLanguage()
        ? QStringLiteral("auto") : m_modelManager->selectedLanguage();
    if (isParakeet) {
        m_parakeetProvider->setLanguage(lang);
    } else {
        // Whisper hallucinates ("Thank you" etc.) with auto-detect (language=nullptr).
        // Only Parakeet handles auto-detection natively — force explicit language for Whisper.
        const QString whisperLang = (lang == QStringLiteral("auto"))
            ? m_modelManager->selectedLanguage() : lang;
        m_whisperProvider->setLanguage(whisperLang.isEmpty() ? QStringLiteral("en") : whisperLang);
    }
    m_whisperProvider->setThreadCount(m_modelManager->threadCount());
    const bool alreadyLoaded = isParakeet
        ? m_parakeetProvider->modelLoaded()
        : m_whisperProvider->modelLoaded();

    if (alreadyLoaded) {
        // Fast path — model already in RAM
        if (isParakeet) m_parakeetProvider->start();
        else m_whisperProvider->start();

        m_loading = false;
        emit isLoadingChanged();
        m_active.store(true, std::memory_order_relaxed);
        emit isActiveChanged();
        resetSilenceTimer();
        qDebug() << "GlobalDictationService: Started instantly (model in RAM)";
    } else {
        // Slow path — load model in background thread
        qDebug() << "GlobalDictationService: Loading model" << useModelId << "from" << modelPath
                 << (isParakeet ? "(Parakeet — may take 30-60s)" : "(Whisper)");

        // Start a timeout timer — if loading takes >90s, abort
        auto* timeoutTimer = new QTimer(this);
        timeoutTimer->setSingleShot(true);
        timeoutTimer->setInterval(90000); // 90 seconds
        connect(timeoutTimer, &QTimer::timeout, this, [this, timeoutTimer]() {
            if (m_loading) {
                qWarning() << "GlobalDictationService: Model loading timed out (90s)";
                m_loading = false;
                emit isLoadingChanged();
                emit errorOccurred(tr("Model loading timed out. The model may be too large for this device, or the file is corrupted. Try a smaller model."));
            }
            timeoutTimer->deleteLater();
        });
        timeoutTimer->start();

        std::thread([this, modelPath, isParakeet]() {
            qDebug() << "GlobalDictationService: Background thread started for model loading";
            bool success = false;
            if (isParakeet)
                success = m_parakeetProvider->loadModel(modelPath);
            else
                success = m_whisperProvider->loadModel(modelPath);
            qDebug() << "GlobalDictationService: loadModel returned:" << success;

            QMetaObject::invokeMethod(this, [this, isParakeet, success]() {
                if (!m_loading) return; // Was cancelled or timed out while loading

                if (!success) {
                    m_loading = false;
                    emit isLoadingChanged();
                    emit errorOccurred(tr("Failed to load model. Try downloading it again."));
                    return;
                }

                if (isParakeet) m_parakeetProvider->start();
                else m_whisperProvider->start();

                m_loading = false;
                emit isLoadingChanged();
                m_active.store(true, std::memory_order_relaxed);
                emit isActiveChanged();
                resetSilenceTimer();
                qDebug() << "GlobalDictationService: Started (model loaded from disk)";
            }, Qt::QueuedConnection);
        }).detach();
    }
}

void GlobalDictationService::stop()
{
    if (!m_active.load(std::memory_order_relaxed)) return;
    if (m_stopping.load(std::memory_order_relaxed)) return;
    m_stopping.store(true, std::memory_order_relaxed);
    emit isStoppingChanged();

    m_silenceTimer.stop();

    const bool isParakeet = m_useParakeet.load(std::memory_order_relaxed);

    // DON'T set m_active=false yet! feedAudio() must keep delivering
    // frames to the provider's ring buffer while we drain it.
    // The UI transition is handled by the overlay's stop button directly.

    std::thread([this, isParakeet]() {
        // Wait 250ms for the last audio frames still in flight from the
        // realtime audio callback to arrive in the provider's ring buffer.
        // Without this, the last ~200ms of speech (3-5 words) are lost.
        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        // NOW stop accepting new audio
        m_active.store(false, std::memory_order_relaxed);

        // Flush remaining audio, then stop provider
        if (isParakeet) {
            m_parakeetProvider->flushRemaining();
            m_parakeetProvider->stop();
        } else {
            m_whisperProvider->flushRemaining();
            m_whisperProvider->stop();
        }

        // Wait for textReady signals (queued via Qt::QueuedConnection) to be
        // processed on the main thread before reading m_currentText.
        // flushRemaining() queues textReady → signal → slot (appendText) = 2 event loop cycles.
        // Without this delay, dictationComplete can fire before appendText runs.
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Finalize on main thread
        QMetaObject::invokeMethod(this, [this]() {
            emit isActiveChanged();

            // If cancelled (clearText was called during stop), skip finalization
            if (m_cancelled.load(std::memory_order_relaxed)) {
                m_stopping.store(false, std::memory_order_relaxed);
                emit isStoppingChanged();
                qDebug() << "GlobalDictationService: Stopped (cancelled)";
                return;
            }

            m_finalText = m_currentText.trimmed();

            // Post-process: apply custom word corrections + snippet expansions
            if (m_postProcessor && !m_finalText.isEmpty())
                m_finalText = m_postProcessor->process(m_finalText);

            // Record dictation statistics
            if (m_dictationStats && !m_finalText.isEmpty())
                m_dictationStats->recordTranscription(m_finalText, m_recordingTimer.elapsed(), m_language);

            emit dictationComplete();

            if (!m_finalText.isEmpty()) {
                copyToClipboard();
                if (m_autoPaste)
                    simulatePaste();
            }

            m_stopping.store(false, std::memory_order_relaxed);
            emit isStoppingChanged();
            qDebug() << "GlobalDictationService: Stopped, text length:" << m_finalText.length();
        }, Qt::QueuedConnection);
    }).detach();
}

void GlobalDictationService::copyToClipboard()
{
    if (auto* clipboard = QGuiApplication::clipboard()) {
        // Use finalText if available (after stop), otherwise currentText (during dictation)
        const QString text = m_finalText.isEmpty() ? m_currentText.trimmed() : m_finalText;
        if (!text.isEmpty()) {
            clipboard->setText(text);
            qDebug() << "GlobalDictationService: Copied to clipboard";
        }
    }
}

void GlobalDictationService::clearText()
{
    m_currentText.clear();
    m_finalText.clear();
    m_cancelled.store(true, std::memory_order_relaxed);
    emit textUpdated();
}

void GlobalDictationService::feedAudio(const float* samples, uint32_t frameCount)
{
    // Called from audio thread — MUST be lock-free
    if (!m_active.load(std::memory_order_relaxed)) return;

    // RMS level for waveform visualization (lock-free)
    float sum = 0.0f;
    for (uint32_t i = 0; i < frameCount; ++i)
        sum += samples[i] * samples[i];
    const float rms = std::sqrt(sum / static_cast<float>(frameCount));
    // Scale up aggressively — typical speech RMS is 0.01-0.1
    const float level = std::min(1.0f, rms * 15.0f);
    const float prev = m_audioLevel.load(std::memory_order_relaxed);

    if (std::abs(level - prev) > 0.01f) {
        m_audioLevel.store(level, std::memory_order_relaxed);
        QMetaObject::invokeMethod(this, [this]() {
            emit audioLevelChanged();
        }, Qt::QueuedConnection);
    }

    if (m_useParakeet.load(std::memory_order_relaxed))
        m_parakeetProvider->feedAudio(samples, frameCount);
    else
        m_whisperProvider->feedAudio(samples, frameCount);
}

bool GlobalDictationService::loadModelIfNeeded()
{
    if (!m_modelManager) {
        emit errorOccurred(tr("No model manager available"));
        return false;
    }

    const QString activeId = m_modelManager->activeModelId();
    const QString modelPath = m_modelManager->modelFilePath(activeId);
    if (modelPath.isEmpty()) {
        emit errorOccurred(tr("No model downloaded. Please download a model in Settings → AI & Tools."));
        return false;
    }

    // Determine engine type (Whisper vs Parakeet)
    const auto models = m_modelManager->availableModels();
    for (const auto& mv : models) {
        auto m = mv.toMap();
        if (m[QStringLiteral("id")].toString() == activeId) {
            m_useParakeet.store(m[QStringLiteral("engine")].toString() == QStringLiteral("parakeet"),
                                std::memory_order_relaxed);
            break;
        }
    }

    // Load model into the appropriate provider
    if (m_useParakeet.load(std::memory_order_relaxed)) {
        if (!m_parakeetProvider->modelLoaded())
            m_parakeetProvider->loadModel(modelPath);
    } else {
        if (!m_whisperProvider->modelLoaded())
            m_whisperProvider->loadModel(modelPath);
    }

    return true;
}

void GlobalDictationService::appendText(const QString& text)
{
    if (m_cancelled.load(std::memory_order_relaxed)) return;
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) return;

    if (m_currentText.isEmpty()) {
        m_currentText = trimmed;
    } else {
        // Deduplicate overlap: streaming chunks overlap by ~1s, so the new
        // chunk may start with words already at the end of m_currentText.
        // Find the longest suffix of m_currentText that matches a prefix of
        // the new text (word-level comparison to avoid partial-word matches).
        const QStringList existingWords = m_currentText.split(' ', Qt::SkipEmptyParts);
        const QStringList newWords = trimmed.split(' ', Qt::SkipEmptyParts);

        int bestOverlap = 0;
        // Try overlap lengths from longest possible down to 1
        const int maxOverlap = qMin(existingWords.size(), newWords.size());
        for (int overlapLen = maxOverlap; overlapLen >= 1; --overlapLen) {
            bool match = true;
            for (int j = 0; j < overlapLen; ++j) {
                if (existingWords[existingWords.size() - overlapLen + j]
                        .compare(newWords[j], Qt::CaseInsensitive) != 0) {
                    match = false;
                    break;
                }
            }
            if (match) {
                bestOverlap = overlapLen;
                break;
            }
        }

        // Append only the non-overlapping part
        if (bestOverlap < newWords.size()) {
            QStringList uniqueWords = newWords.mid(bestOverlap);
            m_currentText.append(' ');
            m_currentText.append(uniqueWords.join(' '));
        }
        // If bestOverlap == newWords.size(), the entire new chunk is a duplicate → skip
    }

    emit textUpdated();

    // Reset silence timeout — user is still speaking
    resetSilenceTimer();
}

void GlobalDictationService::onWhisperTextReady(const QString& text, qint64 /*timestampMs*/)
{
    appendText(text);
}

void GlobalDictationService::onParakeetTextReady(const QString& text, qint64 /*timestampMs*/)
{
    appendText(text);
}

void GlobalDictationService::preloadModel()
{
    if (m_loading) return;
    if (!m_modelManager) return;

    // Find best available model
    QString modelId = m_modelManager->activeModelId();
    QString modelPath = m_modelManager->modelFilePath(modelId);

    // Fallback to any downloaded model
    if (modelPath.isEmpty()) {
        const auto models = m_modelManager->availableModels();
        for (const auto& mv : models) {
            auto m = mv.toMap();
            if (m[QStringLiteral("downloaded")].toBool() && !m[QStringLiteral("locked")].toBool()) {
                modelId = m[QStringLiteral("id")].toString();
                modelPath = m_modelManager->modelFilePath(modelId);
                if (!modelPath.isEmpty()) break;
            }
        }
    }

    if (modelPath.isEmpty()) return; // No model downloaded

    // Determine engine
    const auto models = m_modelManager->availableModels();
    for (const auto& mv : models) {
        auto m = mv.toMap();
        if (m[QStringLiteral("id")].toString() == modelId) {
            m_useParakeet.store(m[QStringLiteral("engine")].toString() == QStringLiteral("parakeet"),
                                std::memory_order_relaxed);
            break;
        }
    }

    const bool isParakeet = m_useParakeet.load(std::memory_order_relaxed);
    const bool alreadyLoaded = isParakeet
        ? m_parakeetProvider->modelLoaded()
        : m_whisperProvider->modelLoaded();

    if (alreadyLoaded) return;

    // Silent pre-load — NO isLoadingChanged signal (don't show overlay)
    m_preloading = true;
    qDebug() << "GlobalDictationService: Pre-loading" << modelId << "in background...";

    std::thread([this, isParakeet]() {
        const QString path = m_modelManager->modelFilePath(m_modelManager->activeModelId());
        bool success = false;
        if (isParakeet)
            success = m_parakeetProvider->loadModel(path);
        else
            success = m_whisperProvider->loadModel(path);

        QMetaObject::invokeMethod(this, [this, success]() {
            m_preloading = false;
            if (success)
                qDebug() << "GlobalDictationService: Model pre-loaded and ready";
            else
                qWarning() << "GlobalDictationService: Pre-load failed (will retry on hotkey)";
        }, Qt::QueuedConnection);
    }).detach();
}

void GlobalDictationService::setAutoPaste(bool enabled)
{
    if (m_autoPaste == enabled) return;
    m_autoPaste = enabled;
    QSettings().setValue(QStringLiteral("dictation/autoPaste"), enabled);
    emit autoPasteChanged();
}

void GlobalDictationService::setOverlayVisible(bool visible)
{
    if (m_overlayVisible == visible) return;
    m_overlayVisible = visible;
    QSettings().setValue(QStringLiteral("dictation/overlayVisible"), visible);
    emit overlayVisibleChanged();
}

void GlobalDictationService::setSilenceTimeoutSecs(int secs)
{
    if (m_silenceTimeoutSecs == secs) return;
    m_silenceTimeoutSecs = qBound(5, secs, 120);
    QSettings().setValue(QStringLiteral("dictation/silenceTimeout"), m_silenceTimeoutSecs);
    emit silenceTimeoutChanged();
}

void GlobalDictationService::resetSilenceTimer()
{
    if (m_silenceTimeoutSecs > 0 && m_active.load(std::memory_order_relaxed))
        m_silenceTimer.start(m_silenceTimeoutSecs * 1000);
}

void GlobalDictationService::simulatePaste()
{
#ifdef __APPLE__
    // Check Accessibility permission first — CGEventPost fails silently without it
    if (!AXIsProcessTrusted()) {
        qWarning() << "GlobalDictationService: Accessibility permission not granted — "
                       "cannot auto-paste. Text is in clipboard, user can Cmd+V manually.";
        emit errorOccurred(tr("Grant Accessibility permission in System Settings → "
                              "Privacy & Security → Accessibility to enable auto-paste."));
        return;
    }

    // Restore the app that was focused when dictation started
    if (m_macWindowHelper)
        m_macWindowHelper->restoreFrontmostApp();

    // 500ms delay: clipboard needs to settle, and the restored app
    // needs to regain focus before receiving the paste event.
    QTimer::singleShot(500, this, [this]() {
        CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
        if (!source) {
            qWarning() << "GlobalDictationService: Failed to create CGEventSource";
            return;
        }

        // Key down: Cmd+V (V = keycode 9 — universal across keyboard layouts)
        CGEventRef keyDown = CGEventCreateKeyboardEvent(source, 9, true);
        if (!keyDown) {
            qWarning() << "GlobalDictationService: Failed to create keyDown event";
            CFRelease(source);
            return;
        }
        CGEventSetFlags(keyDown, kCGEventFlagMaskCommand);
        CGEventPost(kCGAnnotatedSessionEventTap, keyDown);

        // 20ms delay between key down and key up for reliability
        usleep(20000);

        // Key up
        CGEventRef keyUp = CGEventCreateKeyboardEvent(source, 9, false);
        if (!keyUp) {
            qWarning() << "GlobalDictationService: Failed to create keyUp event";
            CFRelease(keyDown);
            CFRelease(source);
            return;
        }
        CGEventSetFlags(keyUp, kCGEventFlagMaskCommand);
        CGEventPost(kCGAnnotatedSessionEventTap, keyUp);

        CFRelease(keyUp);
        CFRelease(keyDown);
        CFRelease(source);

        emit textPasted();
        qDebug() << "GlobalDictationService: Auto-pasted via Cmd+V";
    });
#else
    qDebug() << "GlobalDictationService: Auto-paste not supported on this platform";
#endif
}
