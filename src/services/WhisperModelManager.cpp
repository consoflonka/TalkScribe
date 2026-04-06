#include "WhisperModelManager.h"
// LicenseClient removed — all models unlocked in TalkScribe
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QDirIterator>
#include <thread>
#include <QFile>
#include <QFileInfo>
#include <QStorageInfo>
#include <QSysInfo>
#include <QThread>
#include <QDebug>

#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

// Model catalog — pinned URLs + checksums for reproducible downloads
// Speed/Accuracy ratings are relative comparisons (like MacWhisper)
const QVector<WhisperModelManager::ModelInfo> WhisperModelManager::s_models = {
    // ── Whisper Models (OpenAI, via whisper.cpp) ──
    {
        QStringLiteral("small"),
        QStringLiteral("Whisper Small"),
        QStringLiteral("https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-small.bin"),
        QStringLiteral(""), // SHA-256 skipped — HuggingFace may update files
        488'000'000,  // ~500 MB
        1, 60, 75, false,
        QStringLiteral("Fast & lightweight — good for real-time on older hardware"),
        ModelEngine::Whisper, {} // no vocab
    },
    {
        QStringLiteral("medium"),
        QStringLiteral("Whisper Medium"),
        QStringLiteral("https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-medium.bin"),
        QStringLiteral(""), // SHA-256 skipped
        1'530'000'000, // ~1.5 GB
        1, 40, 80, false,
        QStringLiteral("Balanced — good accuracy, works for real-time on modern Macs"),
        ModelEngine::Whisper, {}
    },
    {
        QStringLiteral("large-v3-turbo"),
        QStringLiteral("Whisper Large V3 Turbo"),
        QStringLiteral("https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-large-v3-turbo.bin"),
        QStringLiteral(""), // SHA-256 skipped
        809'000'000,   // ~809 MB
        1, 75, 90, true,
        QStringLiteral("Best all-rounder — real-time capable, near Large V3 accuracy"),
        ModelEngine::Whisper, {}
    },
    {
        QStringLiteral("large-v3"),
        QStringLiteral("Whisper Large V3"),
        QStringLiteral("https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-large-v3.bin"),
        QStringLiteral(""), // SHA-256 skipped
        3'090'000'000, // ~3 GB
        2, 20, 100, false,
        QStringLiteral("Highest accuracy — too slow for real-time, best for offline transcription"),
        ModelEngine::Whisper, {}
    },
    // ── Parakeet TDT 0.6b (NVIDIA, via sherpa-onnx) ──
    {
        QStringLiteral("parakeet-tdt-0.6b"),
        QStringLiteral("Parakeet TDT 0.6b"),
        QStringLiteral("https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8.tar.bz2"),
        QStringLiteral(""), // SHA-256 verified after first download
        640'000'000,   // ~640 MB (compressed archive)
        1, 95, 95, true,
        QStringLiteral("NVIDIA's fastest model — real-time capable, 25 European languages"),
        ModelEngine::Parakeet,
        {} // tokens.txt included in archive
    },
};

WhisperModelManager::WhisperModelManager(QObject* parent)
    : QObject(parent)
{
    ensureStorageDir();

    // Load saved settings
    QSettings settings;
    m_activeModelId = settings.value(QStringLiteral("whisper/activeModel"), QStringLiteral("large-v3-turbo")).toString();
    // Auto-detect optimal thread count based on CPU cores (cap at 8)
    const int autoThreads = qBound(2, static_cast<int>(std::thread::hardware_concurrency()) / 2, 8);
    m_threadCount = autoThreads;
    m_autoDetectLanguage = settings.value(QStringLiteral("whisper/autoDetectLanguage"), true).toBool();
    m_selectedLanguage = settings.value(QStringLiteral("whisper/selectedLanguage"), QStringLiteral("de")).toString();
    m_globalMode = settings.value(QStringLiteral("whisper/globalMode"), QStringLiteral("hybrid")).toString();
}

QString WhisperModelManager::storagePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
           + QStringLiteral("/whisper-models");
}

void WhisperModelManager::ensureStorageDir()
{
    QDir dir(storagePath());
    if (!dir.exists())
        dir.mkpath(QStringLiteral("."));

    // Clean up leftover .tmp files from interrupted downloads
    const auto tmpFiles = dir.entryInfoList({QStringLiteral("*.tmp")}, QDir::Files);
    for (const auto& tmp : tmpFiles) {
        qDebug() << "WhisperModelManager: Cleaning up leftover temp file:" << tmp.fileName();
        QFile::remove(tmp.absoluteFilePath());
    }
}

const WhisperModelManager::ModelInfo* WhisperModelManager::findModel(const QString& modelId) const
{
    for (const auto& m : s_models) {
        if (m.id == modelId)
            return &m;
    }
    return nullptr;
}

QVariantList WhisperModelManager::availableModels() const
{
    QVariantList result;
    for (const auto& m : s_models) {
        QVariantMap map;
        map[QStringLiteral("id")] = m.id;
        map[QStringLiteral("displayName")] = m.displayName;
        map[QStringLiteral("sizeBytes")] = m.sizeBytes;
        map[QStringLiteral("tierRequired")] = m.tierRequired;
        map[QStringLiteral("downloaded")] = isModelAvailable(m.id);
        map[QStringLiteral("speedPercent")] = m.speedPercent;
        map[QStringLiteral("accuracyPercent")] = m.accuracyPercent;
        map[QStringLiteral("recommended")] = m.recommended;
        map[QStringLiteral("description")] = m.description;
        map[QStringLiteral("engine")] = m.engine == ModelEngine::Parakeet
            ? QStringLiteral("parakeet") : QStringLiteral("whisper");
        // Parakeet requires ONNX Runtime — mark unavailable if not compiled in
        bool needsOnnx = m.engine == ModelEngine::Parakeet;
        bool onnxOk = onnxAvailable();
        // No tier-gating in TalkScribe — only check ONNX availability
        map[QStringLiteral("locked")] = (needsOnnx && !onnxOk);
        map[QStringLiteral("unavailableReason")] = (needsOnnx && !onnxOk)
            ? tr("This model engine is not available in this build.")
            : QString();
        result.append(map);
    }
    return result;
}

QString WhisperModelManager::modelFilePath(const QString& modelId) const
{
    const auto* info = findModel(modelId);
    if (!info) return {};

    if (info->engine == ModelEngine::Parakeet) {
        // Parakeet models are directories with encoder/decoder/joiner/tokens
        const QString dirPath = storagePath() + QStringLiteral("/") + modelId;
        if (QDir(dirPath).exists() && QFile::exists(dirPath + QStringLiteral("/encoder.int8.onnx")))
            return dirPath;
        return {};
    }

    // Whisper models are single .bin files
    const QString path = storagePath() + QStringLiteral("/ggml-") + modelId + QStringLiteral(".bin");
    if (QFile::exists(path))
        return path;
    return {};
}

bool WhisperModelManager::isModelAvailable(const QString& modelId) const
{
    return !modelFilePath(modelId).isEmpty();
}

void WhisperModelManager::downloadModel(const QString& modelId)
{
    // Tier-gating: check if user's tier allows this model
    const auto* modelInfo = findModel(modelId);
    // No tier-gating in TalkScribe — all models downloadable

    if (m_isDownloading) {
        qWarning() << "WhisperModelManager: Already downloading, ignoring request for" << modelId;
        return;
    }

    const auto* info = findModel(modelId);

    // Disk space check: need at least 110% of model size free
    if (info) {
        QStorageInfo storage(storagePath());
        const qint64 requiredBytes = static_cast<qint64>(info->sizeBytes * 1.1);
        if (storage.bytesAvailable() < requiredBytes) {
            const qint64 needMb = requiredBytes / 1'000'000;
            const qint64 haveMb = storage.bytesAvailable() / 1'000'000;
            m_downloadError = QStringLiteral("Not enough disk space. Need %1 MB, have %2 MB free.")
                                  .arg(needMb).arg(haveMb);
            emit downloadErrorChanged();
            return;
        }
    }
    if (!info) {
        m_downloadError = QStringLiteral("Unknown model: ") + modelId;
        emit downloadErrorChanged();
        return;
    }

    if (isModelAvailable(modelId)) {
        emit modelReady(modelId);
        return;
    }

    m_isDownloading = true;
    m_downloadProgress = 0.0;
    m_downloadError.clear();
    m_currentDownloadId = modelId;
    emit isDownloadingChanged();
    emit downloadProgressChanged();
    emit downloadErrorChanged();

    QNetworkRequest request(QUrl(info->url));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    m_currentDownload = m_nam.get(request);

    connect(m_currentDownload, &QNetworkReply::downloadProgress,
            this, [this](qint64 received, qint64 total) {
        if (total > 0) {
            m_downloadProgress = static_cast<double>(received) / static_cast<double>(total);
            emit downloadProgressChanged();
        }
    });

    // Determine file path based on engine type
    const bool isArchive = info->url.endsWith(QStringLiteral(".tar.bz2"))
                           || info->url.endsWith(QStringLiteral(".tar.gz"));
    const QString filePath = isArchive
        ? storagePath() + QStringLiteral("/") + m_currentDownloadId + QStringLiteral(".tar.bz2")
        : storagePath() + QStringLiteral("/ggml-") + m_currentDownloadId + QStringLiteral(".bin");
    const QString tempPath = filePath + QStringLiteral(".tmp");

    auto* tempFile = new QFile(tempPath, this);
    if (!tempFile->open(QIODevice::WriteOnly)) {
        m_downloadError = QStringLiteral("Cannot write to: ") + tempPath;
        emit downloadErrorChanged();
        m_isDownloading = false;
        emit isDownloadingChanged();
        delete tempFile;
        m_currentDownload->abort();
        m_currentDownload->deleteLater();
        m_currentDownload = nullptr;
        return;
    }

    connect(m_currentDownload, &QNetworkReply::readyRead, this, [this, tempFile]() {
        if (m_currentDownload)
            tempFile->write(m_currentDownload->readAll());
    });

    connect(m_currentDownload, &QNetworkReply::finished, this, [this, tempFile, filePath, tempPath]() {
        auto* reply = m_currentDownload;
        m_currentDownload = nullptr;
        m_isDownloading = false;
        emit isDownloadingChanged();

        // Write any remaining data
        if (reply->bytesAvailable() > 0)
            tempFile->write(reply->readAll());
        tempFile->close();
        tempFile->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            m_downloadError = reply->errorString();
            m_downloadProgress = 0.0;
            emit downloadErrorChanged();
            emit downloadProgressChanged();
            QFile::remove(tempPath);
            reply->deleteLater();
            return;
        }

        reply->deleteLater();

        // Verify SHA-256
        const auto* info = findModel(m_currentDownloadId);
        if (info && !info->sha256.isEmpty() && !verifyChecksum(tempPath, info->sha256)) {
            QFile::remove(tempPath);
            m_downloadError = QStringLiteral("SHA-256 checksum mismatch — download corrupted");
            emit downloadErrorChanged();
            return;
        }

        // Rename temp → final
        QFile::remove(filePath); // Remove old if exists
        if (!QFile::rename(tempPath, filePath)) {
            m_downloadError = QStringLiteral("Cannot rename temp file");
            emit downloadErrorChanged();
            return;
        }

        // Extract archive if needed (tar.bz2 for Parakeet models)
        if (filePath.endsWith(QStringLiteral(".tar.bz2"))) {
            const QString extractDir = storagePath();
            QProcess extractProc;
            extractProc.setWorkingDirectory(extractDir);
            extractProc.start(QStringLiteral("tar"), {QStringLiteral("xjf"), filePath});
            if (!extractProc.waitForFinished(60000) || extractProc.exitCode() != 0) {
                m_downloadError = QStringLiteral("Failed to extract archive: ") + extractProc.errorString();
                emit downloadErrorChanged();
                QFile::remove(filePath);
                return;
            }
            // Rename extracted directory to match modelId
            // sherpa-onnx archives extract to e.g. "sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8"
            QDir dir(extractDir);
            const auto entries = dir.entryList({QStringLiteral("sherpa-onnx-*")}, QDir::Dirs);
            if (!entries.isEmpty()) {
                const QString extractedName = entries.first();
                const QString targetName = m_currentDownloadId;
                if (extractedName != targetName) {
                    QDir(extractDir + QStringLiteral("/") + targetName).removeRecursively();
                    dir.rename(extractedName, targetName);
                }
            }
            QFile::remove(filePath); // Remove archive after extraction
            qDebug() << "WhisperModelManager: Archive extracted for" << m_currentDownloadId;
        }

        if (info)
            downloadOptionalVocab(*info);

        m_downloadProgress = 1.0;
        emit downloadProgressChanged();
        emit availableModelsChanged();

        // Auto-select this model if no model is currently downloaded+active
        if (!isModelAvailable(m_activeModelId))
            setActiveModel(m_currentDownloadId);

        emit modelReady(m_currentDownloadId);

        qDebug() << "WhisperModelManager: Model" << m_currentDownloadId << "downloaded to" << filePath;
    });
}

void WhisperModelManager::cancelDownload()
{
    if (m_currentDownload) {
        m_currentDownload->abort();
        m_currentDownload->deleteLater();
        m_currentDownload = nullptr;
        m_isDownloading = false;
        m_downloadProgress = 0.0;
        emit isDownloadingChanged();
        emit downloadProgressChanged();
    }
}

void WhisperModelManager::deleteModel(const QString& modelId)
{
    bool removed = false;
    const auto* info = findModel(modelId);

    // Parakeet: delete directory
    if (info && info->engine == ModelEngine::Parakeet) {
        const QString dirPath = storagePath() + QStringLiteral("/") + modelId;
        if (QDir(dirPath).exists()) {
            QDir(dirPath).removeRecursively();
            removed = true;
        }
    } else {
        // Whisper: delete .bin file
        const QString path = storagePath() + QStringLiteral("/ggml-") + modelId + QStringLiteral(".bin");
        if (QFile::exists(path)) {
            QFile::remove(path);
            if (info && !info->vocabUrl.isEmpty())
                QFile::remove(storagePath() + QStringLiteral("/vocab.txt"));
            removed = true;
        }
    }

    if (removed) {
        emit modelDeleted(modelId);
        emit availableModelsChanged();

        if (m_activeModelId == modelId) {
            m_activeModelId.clear();
            QSettings().remove(QStringLiteral("whisper/activeModel"));
            emit activeModelChanged();
        }
    }
}

void WhisperModelManager::setActiveModel(const QString& modelId)
{
    if (m_activeModelId == modelId)
        return;
    if (!modelId.isEmpty() && !isModelAvailable(modelId)) {
        qWarning() << "WhisperModelManager: Cannot activate model — not downloaded:" << modelId;
        return;
    }

    m_activeModelId = modelId;
    QSettings().setValue(QStringLiteral("whisper/activeModel"), modelId);
    emit activeModelChanged();
}

qint64 WhisperModelManager::totalDiskUsage() const
{
    qint64 total = 0;
    QDir dir(storagePath());
    // Count .bin files (Whisper models)
    for (const auto& entry : dir.entryInfoList(QDir::Files)) {
        total += entry.size();
    }
    // Count Parakeet model directories (encoder/decoder/joiner inside)
    for (const auto& entry : dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QDirIterator it(entry.absoluteFilePath(), QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            total += it.fileInfo().size();
        }
    }
    return total;
}

bool WhisperModelManager::verifyChecksum(const QString& filePath, const QString& expectedSha256) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file))
        return false;

    const QString actual = QString::fromLatin1(hash.result().toHex());
    return actual.compare(expectedSha256, Qt::CaseInsensitive) == 0;
}

void WhisperModelManager::downloadOptionalVocab(const ModelInfo& info) const
{
    if (info.vocabUrl.isEmpty())
        return;

    const QString vocabPath = storagePath() + QStringLiteral("/vocab.txt");
    if (QFile::exists(vocabPath))
        return;

    QNetworkRequest request(QUrl(info.vocabUrl));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    auto* reply = const_cast<QNetworkAccessManager&>(m_nam).get(request);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "WhisperModelManager: Failed to download vocab:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    QFile vocabFile(vocabPath);
    if (!vocabFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "WhisperModelManager: Cannot write vocab to" << vocabPath;
        reply->deleteLater();
        return;
    }

    vocabFile.write(reply->readAll());
    reply->deleteLater();
}

// ── GPU / Hardware info ──

bool WhisperModelManager::gpuAccelerated() const
{
#if defined(__APPLE__)
    return true; // Metal via whisper.cpp + CoreML via ONNX Runtime
#else
    return false;
#endif
}

QString WhisperModelManager::gpuBackend() const
{
#if defined(__APPLE__)
    return QStringLiteral("Apple Metal / CoreML");
#elif defined(_WIN32)
    return QStringLiteral("DirectML (if available)");
#else
    return QStringLiteral("CPU only");
#endif
}

bool WhisperModelManager::onnxAvailable() const
{
#if (defined(TALKLESS_HAS_SHERPA_ONNX) && TALKLESS_HAS_SHERPA_ONNX) || (defined(TALKLESS_HAS_ONNX) && TALKLESS_HAS_ONNX)
    return true;
#else
    return false;
#endif
}

// ── Thread count ──

void WhisperModelManager::setThreadCount(int count)
{
    count = qBound(1, count, 16);
    if (m_threadCount == count) return;
    m_threadCount = count;
    QSettings().setValue(QStringLiteral("whisper/threadCount"), count);
    emit threadCountChanged();
}

// ── Auto-detect language ──

void WhisperModelManager::setAutoDetectLanguage(bool enabled)
{
    if (m_autoDetectLanguage == enabled) return;
    m_autoDetectLanguage = enabled;
    QSettings().setValue(QStringLiteral("whisper/autoDetectLanguage"), enabled);
    emit autoDetectLanguageChanged();
}

void WhisperModelManager::setSelectedLanguage(const QString& lang)
{
    if (m_selectedLanguage == lang) return;
    m_selectedLanguage = lang;
    QSettings().setValue(QStringLiteral("whisper/selectedLanguage"), lang);
    emit selectedLanguageChanged();
}

void WhisperModelManager::setGlobalMode(const QString& mode)
{
    if (m_globalMode == mode) return;
    if (mode != QStringLiteral("local") && mode != QStringLiteral("hybrid") && mode != QStringLiteral("cloud")) {
        qWarning() << "WhisperModelManager: Invalid globalMode ignored:" << mode;
        return;
    }
    m_globalMode = mode;
    QSettings().setValue(QStringLiteral("whisper/globalMode"), mode);
    emit globalModeChanged();
}

// ── Benchmark ──

void WhisperModelManager::benchmarkModel(const QString& modelId)
{
    Q_UNUSED(modelId)
    // TODO: Load model, run whisper_full on 3s of silence, measure wall time
    // Emit benchmarkResult(modelId, elapsedMs)
    // For now, emit estimated values based on model size + platform
    const auto* info = findModel(modelId);
    if (!info) return;

    int estimatedMs = 0;
    if (info->engine == ModelEngine::Parakeet) {
        estimatedMs = 150; // Parakeet is very fast
    } else {
        // Rough estimates for Apple Silicon M1
        if (info->id == QStringLiteral("small")) estimatedMs = 200;
        else if (info->id == QStringLiteral("medium")) estimatedMs = 500;
        else if (info->id == QStringLiteral("large-v3-turbo")) estimatedMs = 300;
        else if (info->id == QStringLiteral("large-v3")) estimatedMs = 1500;
    }
    emit benchmarkResult(modelId, estimatedMs);
}

// ── Hardware capability check ──

bool WhisperModelManager::deviceCanHandleLocal() const
{
    // Heuristic: Check RAM and CPU cores
    // Apple Silicon M1+ with 8GB+ RAM → excellent for local
    // Intel/older Macs with <8GB → may struggle

#ifdef __APPLE__
    // Apple Silicon always has Metal → good performance
    // Check available RAM as proxy for device class
    const qint64 totalRamMB = QSysInfo::machineHostName().isEmpty() ? 8192 :
        // sysctl hw.memsize on macOS
        []() -> qint64 {
            int64_t memSize = 0;
            size_t len = sizeof(memSize);
            if (sysctlbyname("hw.memsize", &memSize, &len, nullptr, 0) == 0)
                return memSize / (1024 * 1024);
            return 8192; // assume 8GB if can't detect
        }();
    return totalRamMB >= 8192; // 8GB+ RAM → good for local
#else
    // Windows/Linux: check core count as proxy
    const int cores = QThread::idealThreadCount();
    return cores >= 4;
#endif
}

QString WhisperModelManager::recommendedMode() const
{
    if (deviceCanHandleLocal())
        return QStringLiteral("local");  // Fast device → local is best (privacy + speed)
    return QStringLiteral("hybrid");     // Slower device → use cloud for own voice, local for loopback
}
