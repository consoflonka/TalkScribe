#include "DictationStatsService.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QDate>
#include <QDebug>
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

DictationStatsService::DictationStatsService(QObject* parent)
    : QObject(parent)
{
}

void DictationStatsService::setStoragePath(const QString& userDir)
{
    m_userDir = userDir;
    load();
}

void DictationStatsService::recordTranscription(const QString& text, qint64 durationMs, const QString& language)
{
    if (text.isEmpty() || m_userDir.isEmpty())
        return;

    const int wordCount = text.split(' ', Qt::SkipEmptyParts).size();
    const QString today = QDate::currentDate().toString(Qt::ISODate);

    // Update totals
    m_totalWords += wordCount;
    m_totalTranscriptions++;
    m_totalDurationMs += durationMs;

    // Update language breakdown
    const QString langKey = language.isEmpty() ? QStringLiteral("unknown") : language;
    m_languageBreakdown[langKey] = m_languageBreakdown.value(langKey, 0).toInt() + wordCount;

    // Update daily stats — find or create today's entry
    bool foundToday = false;
    for (int i = 0; i < m_dailyStatsJson.size(); ++i) {
        auto entry = m_dailyStatsJson[i].toObject();
        if (entry.value("date").toString() == today) {
            entry["words"] = entry.value("words").toInt() + wordCount;
            entry["transcriptions"] = entry.value("transcriptions").toInt() + 1;
            entry["durationMs"] = static_cast<qint64>(entry.value("durationMs").toDouble()) + durationMs;
            // Per-day language breakdown
            auto langs = entry.value("languages").toObject();
            langs[langKey] = langs.value(langKey).toInt() + wordCount;
            entry["languages"] = langs;
            m_dailyStatsJson[i] = entry;
            foundToday = true;
            break;
        }
    }

    if (!foundToday) {
        QJsonObject entry;
        entry["date"] = today;
        entry["words"] = wordCount;
        entry["transcriptions"] = 1;
        entry["durationMs"] = durationMs;
        QJsonObject langs;
        langs[langKey] = wordCount;
        entry["languages"] = langs;
        m_dailyStatsJson.append(entry);
    }

    // Rebuild QVariantList for QML
    m_dailyStats.clear();
    for (const auto& v : m_dailyStatsJson)
        m_dailyStats.append(v.toObject().toVariantMap());

    save();
    emit statsChanged();
}

QString DictationStatsService::estimatedTimeSaved() const
{
    // Estimate: typing at ~150 WPM = 2.5 words/sec
    // Speaking is ~3x faster, so time saved ≈ wordCount * 0.4s
    const qint64 savedMs = static_cast<qint64>(m_totalWords * 400); // 0.4s per word in ms

    const qint64 totalSecs = savedMs / 1000;
    const int hours = static_cast<int>(totalSecs / 3600);
    const int mins = static_cast<int>((totalSecs % 3600) / 60);
    const int secs = static_cast<int>(totalSecs % 60);

    if (hours > 0)
        return QStringLiteral("%1h %2m").arg(hours).arg(mins);
    if (mins > 0)
        return QStringLiteral("%1m %2s").arg(mins).arg(secs);
    return QStringLiteral("%1s").arg(secs);
}

void DictationStatsService::refresh()
{
    load();
    emit statsChanged();
}

void DictationStatsService::resetStats()
{
    m_totalWords = 0;
    m_totalTranscriptions = 0;
    m_totalDurationMs = 0;
    m_dailyStatsJson = QJsonArray();
    m_dailyStats.clear();
    m_languageBreakdown.clear();
    save();
    emit statsChanged();
}

QString DictationStatsService::statsFilePath() const
{
    return m_userDir + QStringLiteral("/dictation_stats.json");
}

void DictationStatsService::load()
{
    if (m_userDir.isEmpty())
        return;

    QFile file(statsFilePath());
    if (!file.exists()) {
        qDebug() << "DictationStatsService: No stats file yet, starting fresh";
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "DictationStatsService: Could not open stats file:" << file.errorString();
        return;
    }

    const auto doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject())
        return;

    const auto root = doc.object();
    m_totalWords = root.value("totalWords").toInt();
    m_totalTranscriptions = root.value("totalTranscriptions").toInt();
    m_totalDurationMs = static_cast<qint64>(root.value("totalDurationMs").toDouble());
    m_dailyStatsJson = root.value("dailyStats").toArray();

    // Rebuild language breakdown
    m_languageBreakdown.clear();
    const auto langObj = root.value("languageBreakdown").toObject();
    for (auto it = langObj.begin(); it != langObj.end(); ++it)
        m_languageBreakdown[it.key()] = it.value().toInt();

    // Rebuild QVariantList for QML
    m_dailyStats.clear();
    for (const auto& v : m_dailyStatsJson)
        m_dailyStats.append(v.toObject().toVariantMap());

    qDebug() << "DictationStatsService: Loaded" << m_totalTranscriptions
             << "transcriptions," << m_totalWords << "words";
}

void DictationStatsService::save() const
{
    if (m_userDir.isEmpty())
        return;

    QDir().mkpath(m_userDir);

    QJsonObject root;
    root["totalWords"] = m_totalWords;
    root["totalTranscriptions"] = m_totalTranscriptions;
    root["totalDurationMs"] = static_cast<double>(m_totalDurationMs);
    root["dailyStats"] = m_dailyStatsJson;

    // Save language breakdown
    QJsonObject langObj;
    for (auto it = m_languageBreakdown.constBegin(); it != m_languageBreakdown.constEnd(); ++it)
        langObj[it.key()] = it.value().toInt();
    root["languageBreakdown"] = langObj;

    // Atomic write: tmp → rename
    const QString path = statsFilePath();
    const QString tmpPath = path + QStringLiteral(".tmp");

    QFile file(tmpPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "DictationStatsService: Could not write stats:" << file.errorString();
        return;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    file.flush();
#ifdef Q_OS_UNIX
    fsync(file.handle());
#endif
    file.close();

    // Atomic rename
    QFile::remove(path);
    QFile::rename(tmpPath, path);
}
