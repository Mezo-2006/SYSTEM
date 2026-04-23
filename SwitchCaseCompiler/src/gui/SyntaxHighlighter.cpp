#include "SyntaxHighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Keyword format (VS Code style: light blue)
    keywordFormat.setForeground(QColor("#569CD6"));
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns = {
        "\\bswitch\\b", "\\bcase\\b", "\\bdefault\\b", "\\bbreak\\b", "\\breturn\\b",
        "\\bif\\b", "\\belse\\b", "\\bcontinue\\b"
    };
    for (const QString& pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.push_back(rule);
    }

    // Type keywords format (cyan-blue)
    typeFormat.setForeground(QColor("#4EC9B0"));
    typeFormat.setFontWeight(QFont::Bold);
    QStringList typePatterns = {
        "\\bint\\b", "\\bstring\\b", "\\bvoid\\b", "\\bchar\\b", "\\bbool\\b"
    };
    for (const QString& pattern : typePatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = typeFormat;
        highlightingRules.push_back(rule);
    }

    // I/O keywords format (purple-pink)
    ioFormat.setForeground(QColor("#C586C0"));
    ioFormat.setFontWeight(QFont::Bold);
    QStringList ioPatterns = {
        "\\bcout\\b", "\\bcin\\b", "\\bendl\\b",
        "\\busing\\b", "\\bnamespace\\b", "\\bstd\\b",
        "\\binclude\\b", "\\biomanip\\b", "\\biostream\\b"
    };
    for (const QString& pattern : ioPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = ioFormat;
        highlightingRules.push_back(rule);
    }

    // Function format (light yellow)
    functionFormat.setForeground(QColor("#DCDCAA"));
    rule.pattern = QRegularExpression("\\b[A-Za-z_][A-Za-z0-9_]*(?=\\s*\\()");
    rule.format = functionFormat;
    highlightingRules.push_back(rule);

    // Number format (light green)
    numberFormat.setForeground(QColor("#B5CEA8"));
    rule.pattern = QRegularExpression("\\b[0-9]+\\.?[0-9]*\\b");
    rule.format = numberFormat;
    highlightingRules.push_back(rule);

    // String format (salmon/orange)
    stringFormat.setForeground(QColor("#CE9178"));
    rule.pattern = QRegularExpression("\".*?\"");
    rule.format = stringFormat;
    highlightingRules.push_back(rule);

    // Operator format (white/default)
    operatorFormat.setForeground(QColor("#D4D4D4"));
    QStringList operatorPatterns = {
        "<<", ">>", "\\+", "-", "\\*", "/", "=", "==", "!=", "<", ">", "<=", ">="
    };
    for (const QString& pattern : operatorPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = operatorFormat;
        highlightingRules.push_back(rule);
    }

    // Single line comment format (green)
    commentFormat.setForeground(QColor("#6A9955"));
    commentFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = commentFormat;
    highlightingRules.push_back(rule);

    // Preprocessor directives (magenta)
    QTextCharFormat preprocessorFormat;
    preprocessorFormat.setForeground(QColor("#C586C0"));
    preprocessorFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format = preprocessorFormat;
    highlightingRules.push_back(rule);
}

void SyntaxHighlighter::highlightBlock(const QString& text)
{
    // Apply all rules
    for (const HighlightingRule& rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Multi-line comment handling
    setCurrentBlockState(0);

    QRegularExpression startExpression("/\\*");
    QRegularExpression endExpression("\\*/");

    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = text.indexOf(startExpression);
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch match = endExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, commentFormat);
        startIndex = text.indexOf(startExpression, startIndex + commentLength);
    }
}
