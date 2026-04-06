#include "TextPostProcessor.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QRegularExpression>
#include <algorithm>

TextPostProcessor::TextPostProcessor(QObject* parent)
    : QObject(parent)
{
    loadFromSettings();
}

void TextPostProcessor::setCustomWords(const QJsonArray& words)
{
    m_customWords.clear();
    m_customWords.reserve(words.size());
    for (const auto& v : words) {
        const QString w = v.toString().trimmed();
        if (!w.isEmpty())
            m_customWords.append(w);
    }
}

void TextPostProcessor::setSnippets(const QJsonArray& snippets)
{
    m_snippets.clear();
    m_snippets.reserve(snippets.size());
    for (const auto& v : snippets) {
        const auto obj = v.toObject();
        const QString trigger = obj.value("trigger").toString().trimmed();
        const QString text = obj.value("text").toString();
        if (!trigger.isEmpty())
            m_snippets.append({trigger, text});
    }
    // Sort by trigger length descending so longest match wins
    std::sort(m_snippets.begin(), m_snippets.end(),
              [](const auto& a, const auto& b) {
                  return a.first.length() > b.first.length();
              });
}

QString TextPostProcessor::process(const QString& rawText) const
{
    if (rawText.isEmpty())
        return rawText;

    QString result = rawText;

    if (!m_customWords.isEmpty())
        result = applyCustomWords(result);

    if (!m_snippets.isEmpty())
        result = applySnippets(result);

    return result;
}

// ── QML-accessible API (TalkScribe addition) ──

QVariantList TextPostProcessor::customWords() const
{
    QVariantList result;
    for (const auto& w : m_customWords)
        result.append(w);
    return result;
}

void TextPostProcessor::addCustomWord(const QString& word)
{
    const QString trimmed = word.trimmed();
    if (trimmed.isEmpty() || m_customWords.contains(trimmed))
        return;
    m_customWords.append(trimmed);
    saveToSettings();
    emit customWordsChanged();
}

void TextPostProcessor::removeCustomWord(int index)
{
    if (index < 0 || index >= m_customWords.size())
        return;
    m_customWords.removeAt(index);
    saveToSettings();
    emit customWordsChanged();
}

QVariantList TextPostProcessor::snippetsList() const
{
    QVariantList result;
    for (const auto& [trigger, text] : m_snippets) {
        QVariantMap map;
        map["trigger"] = trigger;
        map["text"] = text;
        result.append(map);
    }
    return result;
}

void TextPostProcessor::addSnippet(const QString& trigger, const QString& text)
{
    const QString t = trigger.trimmed();
    if (t.isEmpty())
        return;
    m_snippets.append({t, text});
    std::sort(m_snippets.begin(), m_snippets.end(),
              [](const auto& a, const auto& b) {
                  return a.first.length() > b.first.length();
              });
    saveToSettings();
    emit snippetsChanged();
}

void TextPostProcessor::updateSnippet(int index, const QString& trigger, const QString& text)
{
    if (index < 0 || index >= m_snippets.size())
        return;
    m_snippets[index] = {trigger.trimmed(), text};
    std::sort(m_snippets.begin(), m_snippets.end(),
              [](const auto& a, const auto& b) {
                  return a.first.length() > b.first.length();
              });
    saveToSettings();
    emit snippetsChanged();
}

void TextPostProcessor::removeSnippet(int index)
{
    if (index < 0 || index >= m_snippets.size())
        return;
    m_snippets.removeAt(index);
    saveToSettings();
    emit snippetsChanged();
}

void TextPostProcessor::loadFromSettings()
{
    // Load custom words
    const auto wordsJson = m_settings.value("dictation/customWords").toByteArray();
    if (!wordsJson.isEmpty()) {
        const auto arr = QJsonDocument::fromJson(wordsJson).array();
        setCustomWords(arr);
    }

    // Load snippets
    const auto snippetsJson = m_settings.value("dictation/snippets").toByteArray();
    if (!snippetsJson.isEmpty()) {
        const auto arr = QJsonDocument::fromJson(snippetsJson).array();
        setSnippets(arr);
    }
}

void TextPostProcessor::saveToSettings()
{
    // Save custom words
    QJsonArray wordsArr;
    for (const auto& w : m_customWords)
        wordsArr.append(w);
    m_settings.setValue("dictation/customWords",
                        QJsonDocument(wordsArr).toJson(QJsonDocument::Compact));

    // Save snippets
    QJsonArray snippetsArr;
    for (const auto& [trigger, text] : m_snippets) {
        QJsonObject obj;
        obj["trigger"] = trigger;
        obj["text"] = text;
        snippetsArr.append(obj);
    }
    m_settings.setValue("dictation/snippets",
                        QJsonDocument(snippetsArr).toJson(QJsonDocument::Compact));
}

// ── Original processing logic ──

QString TextPostProcessor::applyCustomWords(const QString& text) const
{
    QStringList words = text.split(' ', Qt::SkipEmptyParts);
    bool changed = false;

    for (int i = 0; i < words.size(); ++i) {
        const QString& word = words[i];

        if (word.length() < 3)
            continue;

        QString clean = word;
        QString trailing;
        while (!clean.isEmpty() && clean.back().isPunct()) {
            trailing.prepend(clean.back());
            clean.chop(1);
        }
        if (clean.isEmpty())
            continue;

        double bestDist = 1.0;
        int bestIdx = -1;

        for (int j = 0; j < m_customWords.size(); ++j) {
            const QString& target = m_customWords[j];

            const int lenDiff = qAbs(clean.length() - target.length());
            if (lenDiff > qMax(2, target.length() * 2 / 5))
                continue;

            const double dist = normalizedLevenshtein(clean.toLower(), target.toLower());
            if (dist < bestDist) {
                bestDist = dist;
                bestIdx = j;
            }
        }

        const double threshold = (clean.length() <= 4) ? 0.15 : 0.25;
        if (bestIdx >= 0 && bestDist < threshold && bestDist > 0.0) {
            words[i] = m_customWords[bestIdx] + trailing;
            changed = true;
        } else if (bestIdx >= 0 && bestDist == 0.0 && clean != m_customWords[bestIdx]) {
            words[i] = m_customWords[bestIdx] + trailing;
            changed = true;
        }
    }

    return changed ? words.join(' ') : text;
}

QString TextPostProcessor::applySnippets(const QString& text) const
{
    QString result = text;

    for (const auto& [trigger, replacement] : m_snippets) {
        int pos = 0;
        while (true) {
            pos = result.indexOf(trigger, pos, Qt::CaseInsensitive);
            if (pos < 0)
                break;

            const bool leftOk = (pos == 0 || !result[pos - 1].isLetterOrNumber());
            const int endPos = pos + trigger.length();
            const bool rightOk = (endPos >= result.length() || !result[endPos].isLetterOrNumber());

            if (leftOk && rightOk) {
                result.replace(pos, trigger.length(), replacement);
                pos += replacement.length();
            } else {
                pos += trigger.length();
            }
        }
    }

    return result;
}

double TextPostProcessor::normalizedLevenshtein(const QString& a, const QString& b)
{
    if (a == b) return 0.0;
    if (a.isEmpty()) return 1.0;
    if (b.isEmpty()) return 1.0;

    const int m = a.length();
    const int n = b.length();

    QVector<int> prev(n + 1);
    QVector<int> curr(n + 1);

    for (int j = 0; j <= n; ++j)
        prev[j] = j;

    for (int i = 1; i <= m; ++i) {
        curr[0] = i;
        for (int j = 1; j <= n; ++j) {
            const int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            curr[j] = std::min({prev[j] + 1, curr[j - 1] + 1, prev[j - 1] + cost});
        }
        std::swap(prev, curr);
    }

    return static_cast<double>(prev[n]) / qMax(m, n);
}
