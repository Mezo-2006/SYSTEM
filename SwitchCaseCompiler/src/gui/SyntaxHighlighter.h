#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <vector>

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit SyntaxHighlighter(QTextDocument* parent = nullptr);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    std::vector<HighlightingRule> highlightingRules;

    // Formats for different token types
    QTextCharFormat keywordFormat;
    QTextCharFormat typeFormat;
    QTextCharFormat ioFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat operatorFormat;
};

#endif // SYNTAXHIGHLIGHTER_H
