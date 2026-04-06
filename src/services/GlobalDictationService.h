#pragma once

#include <QObject>
#include <QClipboard>
#include <QGuiApplication>
#include <QTimer>
#include <QString>
#include <QElapsedTimer>
#include <atomic>
#include <memory>

class LocalWhisperProvider;
class ParakeetProvider;
class WhisperModelManager;
class MacWindowHelper;
class TextPostProcessor;
class DictationStatsService;

/**
 * @brief Global anywhere-dictation via hotkey.
 *
 * Press hotkey → start recording → live transcription via local Whisper/Parakeet
 * → press again → stop → final text copied to clipboard.
 *
 * Audio is fed from AudioEngine's capture callback (lock-free ring buffer).
 */
class GlobalDictationService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool isStopping READ isStopping NOTIFY isStoppingChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool autoPaste READ autoPaste WRITE setAutoPaste NOTIFY autoPasteChanged)
    Q_PROPERTY(bool overlayVisible READ overlayVisible WRITE setOverlayVisible NOTIFY overlayVisibleChanged)
    Q_PROPERTY(int silenceTimeoutSecs READ silenceTimeoutSecs WRITE setSilenceTimeoutSecs NOTIFY silenceTimeoutChanged)
    Q_PROPERTY(QString currentText READ currentText NOTIFY textUpdated)
    Q_PROPERTY(QString finalText READ finalText NOTIFY dictationComplete)
    Q_PROPERTY(qreal audioLevel READ audioLevel NOTIFY audioLevelChanged)

public:
    explicit GlobalDictationService(QObject* parent = nullptr);
    ~GlobalDictationService() override;

    [[nodiscard]] bool isActive() const { return m_active.load(std::memory_order_relaxed); }
    [[nodiscard]] bool isStopping() const { return m_stopping.load(std::memory_order_relaxed); }
    [[nodiscard]] bool isLoading() const { return m_loading; }
    [[nodiscard]] bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled);
    [[nodiscard]] bool autoPaste() const { return m_autoPaste; }
    [[nodiscard]] bool overlayVisible() const { return m_overlayVisible; }
    [[nodiscard]] int silenceTimeoutSecs() const { return m_silenceTimeoutSecs; }
    [[nodiscard]] QString currentText() const { return m_currentText; }
    [[nodiscard]] QString finalText() const { return m_finalText; }
    [[nodiscard]] qreal audioLevel() const { return static_cast<qreal>(m_audioLevel.load(std::memory_order_relaxed)); }

    void setAutoPaste(bool enabled);
    void setOverlayVisible(bool visible);
    void setSilenceTimeoutSecs(int secs);

    /// Feed audio from AudioEngine capture callback (MUST be lock-free)
    void feedAudio(const float* samples, uint32_t frameCount);

    /// Toggle dictation on/off (called from hotkey)
    Q_INVOKABLE void toggle();
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    /// Pre-load model in background (call after login for instant dictation)
    Q_INVOKABLE void preloadModel();

    /// Copy current text to clipboard
    Q_INVOKABLE void copyToClipboard();

    /// Clear accumulated text
    Q_INVOKABLE void clearText();

    /// Request overlay position reset (emits signal for QML)
    Q_INVOKABLE void requestOverlayPositionReset() { emit overlayPositionResetRequested(); }

    /// Set model manager for model loading
    void setModelManager(WhisperModelManager* mgr) { m_modelManager = mgr; }

    /// Set MacWindowHelper for save/restore frontmost app
    void setMacWindowHelper(MacWindowHelper* helper) { m_macWindowHelper = helper; }

    /// Set text post-processor for custom words & snippets
    void setTextPostProcessor(TextPostProcessor* proc) { m_postProcessor = proc; }

    /// Set dictation stats service for tracking transcription metrics
    void setDictationStatsService(DictationStatsService* stats) { m_dictationStats = stats; }

    /// Set language for transcription (e.g. "de", "en", "es")
    void setLanguage(const QString& code) { m_language = code; }

signals:
    void isActiveChanged();
    void isStoppingChanged();
    void isLoadingChanged();
    void enabledChanged();
    void autoPasteChanged();
    void overlayVisibleChanged();
    void silenceTimeoutChanged();
    void textUpdated();
    void dictationComplete();
    void textPasted();
    void errorOccurred(const QString& error);
    void audioLevelChanged();
    void overlayPositionResetRequested();

private slots:
    void onWhisperTextReady(const QString& text, qint64 timestampMs);
    void onParakeetTextReady(const QString& text, qint64 timestampMs);

private:
    bool loadModelIfNeeded();
    void appendText(const QString& text);
    void simulatePaste();
    void resetSilenceTimer();

    std::unique_ptr<LocalWhisperProvider> m_whisperProvider;
    std::unique_ptr<ParakeetProvider> m_parakeetProvider;
    std::atomic<bool> m_useParakeet{false};

    WhisperModelManager* m_modelManager = nullptr;
    MacWindowHelper* m_macWindowHelper = nullptr;
    TextPostProcessor* m_postProcessor = nullptr;
    DictationStatsService* m_dictationStats = nullptr;
    QElapsedTimer m_recordingTimer;

    std::atomic<bool> m_active{false};
    std::atomic<bool> m_stopping{false};
    std::atomic<bool> m_cancelled{false};
    bool m_loading = false;
    bool m_preloading = false;
    bool m_enabled = false;
    bool m_autoPaste = true;
    bool m_overlayVisible = true;
    int m_silenceTimeoutSecs = 30;
    QTimer m_silenceTimer;
    QString m_currentText;
    QString m_finalText;
    QString m_language = QStringLiteral("de");
    std::atomic<float> m_audioLevel{0.0f};
};
