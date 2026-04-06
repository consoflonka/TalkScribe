#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QVariantList>
#include <QCryptographicHash>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QEventLoop>

/**
 * @brief Manages whisper.cpp model downloads, storage and selection.
 *
 * Models are stored in AppDataLocation/whisper-models/ and downloaded
 * on-demand from HuggingFace with SHA-256 verification.
 */
class WhisperModelManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList availableModels READ availableModels NOTIFY availableModelsChanged)
    Q_PROPERTY(QString activeModelId READ activeModelId WRITE setActiveModel NOTIFY activeModelChanged)
    Q_PROPERTY(bool isDownloading READ isDownloading NOTIFY isDownloadingChanged)
    Q_PROPERTY(double downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)
    Q_PROPERTY(QString downloadError READ downloadError NOTIFY downloadErrorChanged)
    Q_PROPERTY(bool gpuAccelerated READ gpuAccelerated CONSTANT)
    Q_PROPERTY(QString gpuBackend READ gpuBackend CONSTANT)
    Q_PROPERTY(int threadCount READ threadCount WRITE setThreadCount NOTIFY threadCountChanged)
    Q_PROPERTY(bool autoDetectLanguage READ autoDetectLanguage WRITE setAutoDetectLanguage NOTIFY autoDetectLanguageChanged)
    Q_PROPERTY(bool onnxAvailable READ onnxAvailable CONSTANT)
    Q_PROPERTY(QString globalMode READ globalMode WRITE setGlobalMode NOTIFY globalModeChanged)
    Q_PROPERTY(QString selectedLanguage READ selectedLanguage WRITE setSelectedLanguage NOTIFY selectedLanguageChanged)

public:
    explicit WhisperModelManager(QObject* parent = nullptr);
    ~WhisperModelManager() override = default;

    enum class ModelEngine { Whisper, Parakeet };

    struct ModelInfo {
        QString id;
        QString displayName;
        QString url;
        QString sha256;
        qint64 sizeBytes;
        int tierRequired;     // 1 = Pro, 2 = Business
        int speedPercent;     // 0-100 relative speed rating
        int accuracyPercent;  // 0-100 relative accuracy rating
        bool recommended;     // Show "Recommended" badge
        QString description;  // Short description text
        ModelEngine engine;   // Which runtime to use
        QString vocabUrl;     // Optional: vocab.txt URL (for SentencePiece models like Parakeet)
    };

    [[nodiscard]] QVariantList availableModels() const;
    [[nodiscard]] QString activeModelId() const { return m_activeModelId; }
    [[nodiscard]] bool isDownloading() const { return m_isDownloading; }
    [[nodiscard]] double downloadProgress() const { return m_downloadProgress; }
    [[nodiscard]] QString downloadError() const { return m_downloadError; }

    /// Full file path for a downloaded model, or empty if not available
    [[nodiscard]] QString modelFilePath(const QString& modelId) const;

    /// Check if a model is already downloaded and verified
    [[nodiscard]] Q_INVOKABLE bool isModelAvailable(const QString& modelId) const;

    /// Download a model (async, emits progress signals)
    Q_INVOKABLE void downloadModel(const QString& modelId);

    /// Cancel an active download
    Q_INVOKABLE void cancelDownload();

    // No license gating in TalkScribe — all models unlocked

    /// Delete a downloaded model from disk
    Q_INVOKABLE void deleteModel(const QString& modelId);

    /// Set the active model for transcription
    Q_INVOKABLE void setActiveModel(const QString& modelId);

    /// Disk space used by all downloaded models
    [[nodiscard]] Q_INVOKABLE qint64 totalDiskUsage() const;

    // ── GPU / Hardware info ──
    [[nodiscard]] bool gpuAccelerated() const;
    [[nodiscard]] QString gpuBackend() const;
    [[nodiscard]] bool onnxAvailable() const;

    // ── Thread count (configurable, persisted) ──
    [[nodiscard]] int threadCount() const { return m_threadCount; }
    Q_INVOKABLE void setThreadCount(int count);

    // ── Auto-detect language ──
    [[nodiscard]] bool autoDetectLanguage() const { return m_autoDetectLanguage; }
    Q_INVOKABLE void setAutoDetectLanguage(bool enabled);

    // ── Selected language (when auto-detect is off) ──
    [[nodiscard]] QString selectedLanguage() const { return m_selectedLanguage; }
    Q_INVOKABLE void setSelectedLanguage(const QString& lang);

    // ── Global transcription mode (local/hybrid/cloud) ──
    [[nodiscard]] QString globalMode() const { return m_globalMode; }
    Q_INVOKABLE void setGlobalMode(const QString& mode);

    // ── Benchmark ──
    /// Run a quick benchmark on a model (returns ms per 3s chunk)
    Q_INVOKABLE void benchmarkModel(const QString& modelId);

    /// Check if this device can handle local transcription well
    /// Returns true if estimated inference < 2s for 3s chunk (real-time capable)
    Q_INVOKABLE bool deviceCanHandleLocal() const;

    /// Recommended mode based on hardware: "local" for fast devices, "hybrid" for slower ones
    Q_INVOKABLE QString recommendedMode() const;

signals:
    void availableModelsChanged();
    void activeModelChanged();
    void isDownloadingChanged();
    void downloadProgressChanged();
    void downloadErrorChanged();
    void modelReady(const QString& modelId);
    void modelDeleted(const QString& modelId);
    void threadCountChanged();
    void autoDetectLanguageChanged();
    void selectedLanguageChanged();
    void globalModeChanged();
    void benchmarkResult(const QString& modelId, int msPerChunk);

private:
    void ensureStorageDir();
    QString storagePath() const;
    const ModelInfo* findModel(const QString& modelId) const;
    bool verifyChecksum(const QString& filePath, const QString& expectedSha256) const;
    void downloadOptionalVocab(const ModelInfo& info) const;

    QNetworkAccessManager m_nam;
    QNetworkReply* m_currentDownload = nullptr;
    QString m_activeModelId;
    bool m_isDownloading = false;
    double m_downloadProgress = 0.0;
    QString m_downloadError;
    QString m_currentDownloadId;
    // LicenseClient removed — all models unlocked
    int m_threadCount = 4;
    bool m_autoDetectLanguage = true;
    QString m_selectedLanguage = QStringLiteral("de");
    QString m_globalMode = QStringLiteral("hybrid");

    static const QVector<ModelInfo> s_models;
};
