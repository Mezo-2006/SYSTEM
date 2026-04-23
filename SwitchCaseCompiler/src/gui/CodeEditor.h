#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include <QCompleter>
#include <QStringListModel>
#include <QTextEdit>
#include <vector>
#include <string>

class LineNumberArea;
class SyntaxHighlighter;

// Error information for underlining
struct EditorError {
    int line;
    int column;
    int endColumn;
    std::string message;
    
    EditorError(int l, int c, int ec, const std::string& msg)
        : line(l), column(c), endColumn(ec), message(msg) {}
};

/**
 * @class CodeEditor
 * @brief Professional code editor with line numbers, syntax highlighting, and smart features
 *
 * Features:
 * - Line number gutter on left
 * - Real-time syntax highlighting
 * - Error underlining (red squiggle under errors)
 * - Error tooltips on hover
 * - Undo/Redo (Ctrl+Z / Ctrl+Y)
 * - Auto-complete (Ctrl+Space)
 * - Smart indent
 * - Bracket matching
 */
class CodeEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit CodeEditor(QWidget* parent = nullptr);

    // Draw the line number area
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth();

    // Error management
    void setErrors(const std::vector<EditorError>& errors);
    void clearErrors();
    const std::vector<EditorError>& getErrors() const { return errors; }
    void setExtraSelectionsMerged(const QList<QTextEdit::ExtraSelection>& customSelections);

    // Get syntax highlighter
    SyntaxHighlighter* getSyntaxHighlighter() const { return syntaxHighlighter; }

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightErrorLine();
    void updateLineNumberArea(const QRect& rect, int dy);
    void onAutoCompleteActivated(const QString& text);
    void updateAutoCompleteModel();

private:
    void setupAutoComplete();
    void insertMatchingBracket(QChar openBracket);
    void smartIndent();
    void highlightMatchingBrackets();
    bool findMatchingBracket(int& startPos, int& endPos);
    
    LineNumberArea* lineNumberArea;
    SyntaxHighlighter* syntaxHighlighter;
    std::vector<EditorError> errors;
    QCompleter* autoCompleter;
    QStringListModel* completeModel;
    
    // For error tooltips
    int hoveredErrorIndex = -1;
    QString errorTooltip;
    
    // Switch-case keywords for auto-complete
    static const QStringList keywordSuggestions;
};

// Helper class for rendering line numbers
class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(CodeEditor* editor) : QWidget(editor), codeEditor(editor) {}

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    CodeEditor* codeEditor;
};

#endif // CODEEDITOR_H
