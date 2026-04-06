#pragma once

#include <QObject>
#include <QJsonArray>
#include <QSettings>
#include <QString>
#include <QVector>
#include <QPair>
#include <QVariantList>

/**
 * @brief Post-processes transcribed text: custom word correction + snippet expansion.
 *
 * In TalkScribe this also serves as the QML-accessible manager for custom words
 * and snippets (replacing TalkLess's SoundboardService role).
 */
class TextPostProcessor : public QObject
{
    Q_OBJECT

public:
    explicit TextPostProcessor(QObject* parent = nullptr);

    /// Update custom words list from settings
    void setCustomWords(const QJsonArray& words);

    /// Update snippets list from settings
    void setSnippets(const QJsonArray& snippets);

    /// Process text: apply custom word corrections, then snippet expansions
    [[nodiscard]] QString process(const QString& rawText) const;

    // ── QML-accessible API (TalkScribe addition) ──

    /// Get custom words as QVariantList of strings
    Q_INVOKABLE QVariantList customWords() const;

    /// Add a custom word
    Q_INVOKABLE void addCustomWord(const QString& word);

    /// Remove custom word by index
    Q_INVOKABLE void removeCustomWord(int index);

    /// Get snippets as QVariantList of {trigger, text}
    Q_INVOKABLE QVariantList snippetsList() const;

    /// Add a snippet
    Q_INVOKABLE void addSnippet(const QString& trigger, const QString& text);

    /// Update a snippet at index
    Q_INVOKABLE void updateSnippet(int index, const QString& trigger, const QString& text);

    /// Remove snippet by index
    Q_INVOKABLE void removeSnippet(int index);

signals:
    void customWordsChanged();
    void snippetsChanged();

private:
    [[nodiscard]] QString applyCustomWords(const QString& text) const;
    [[nodiscard]] QString applySnippets(const QString& text) const;

    /// Normalized Levenshtein distance (0.0 = identical, 1.0 = completely different)
    [[nodiscard]] static double normalizedLevenshtein(const QString& a, const QString& b);

    void loadFromSettings();
    void saveToSettings();

    QVector<QString> m_customWords;
    // Sorted by trigger length descending (longest match first)
    QVector<QPair<QString, QString>> m_snippets; // trigger → replacement
    QSettings m_settings;
};
