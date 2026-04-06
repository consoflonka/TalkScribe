#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantList>
#include <QVariantMap>
#include <QString>

/**
 * @brief Tracks dictation usage statistics: word count, transcriptions, time saved.
 *
 * Stats are stored locally as dictation_stats.json in the user's data directory.
 * Not part of AppSettings to avoid bloating the settings sync.
 */
class DictationStatsService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int totalWords READ totalWords NOTIFY statsChanged)
    Q_PROPERTY(int totalTranscriptions READ totalTranscriptions NOTIFY statsChanged)
    Q_PROPERTY(qint64 totalDurationMs READ totalDurationMs NOTIFY statsChanged)
    Q_PROPERTY(QString estimatedTimeSaved READ estimatedTimeSaved NOTIFY statsChanged)
    Q_PROPERTY(QVariantList dailyStats READ dailyStats NOTIFY statsChanged)
    Q_PROPERTY(QVariantMap languageBreakdown READ languageBreakdown NOTIFY statsChanged)

public:
    explicit DictationStatsService(QObject* parent = nullptr);

    /// Set storage directory (called after user login)
    void setStoragePath(const QString& userDir);

    /// Record a completed transcription
    void recordTranscription(const QString& text, qint64 durationMs, const QString& language = QStringLiteral("de"));

    /// Reload stats from disk
    Q_INVOKABLE void refresh();

    /// Reset all stats
    Q_INVOKABLE void resetStats();

    [[nodiscard]] int totalWords() const { return m_totalWords; }
    [[nodiscard]] int totalTranscriptions() const { return m_totalTranscriptions; }
    [[nodiscard]] qint64 totalDurationMs() const { return m_totalDurationMs; }
    [[nodiscard]] QString estimatedTimeSaved() const;
    [[nodiscard]] QVariantList dailyStats() const { return m_dailyStats; }
    [[nodiscard]] QVariantMap languageBreakdown() const { return m_languageBreakdown; }

signals:
    void statsChanged();

private:
    void load();
    void save() const;
    [[nodiscard]] QString statsFilePath() const;

    QString m_userDir;
    int m_totalWords = 0;
    int m_totalTranscriptions = 0;
    qint64 m_totalDurationMs = 0;
    QVariantList m_dailyStats;
    QVariantMap m_languageBreakdown;
    QJsonArray m_dailyStatsJson; // Internal storage for save/load
};
