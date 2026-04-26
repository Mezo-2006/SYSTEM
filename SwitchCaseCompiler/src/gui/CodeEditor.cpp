#include "CodeEditor.h"
#include "SyntaxHighlighter.h"
#include <QPainter>
#include <QTextBlock>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QToolTip>
#include <QAbstractTextDocumentLayout>
#include <QFont>
#include <QColor>
#include <algorithm>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>

CodeEditor::CodeEditor(QWidget* parent)
    : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);
    syntaxHighlighter = new SyntaxHighlighter(document());

    // Set font to monospace
    QFont font("JetBrains Mono", 11);
    if (!font.exactMatch()) {
        font = QFont("Consolas", 11);
        if (!font.exactMatch()) {
            font = QFont("Courier New", 11);
        }
    }
    setFont(font);
    setTabStopDistance(4 * fontMetrics().horizontalAdvance(' '));

    // Professional dark theme
    setStyleSheet(
        "CodeEditor {"
        "  background-color: #0B132B;"
        "  color: #E0E1DD;"
        "  border: none;"
        "  selection-background-color: #3A506B;"
        "}"
    );

    // Connect signals
    connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::highlightErrorLine);
    connect(this, &QPlainTextEdit::textChanged, this, &CodeEditor::updateAutoCompleteModel);
    
    // Setup auto-complete
    setupAutoComplete();
    updateAutoCompleteModel();

    updateLineNumberAreaWidth(0);
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int newBlockCount)
{
    Q_UNUSED(newBlockCount);
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect& rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height());
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor("#1A1D27"));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + blockBoundingGeometry(block).height();

    painter.setFont(font());
    painter.setPen(QColor("#9199B8"));

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.drawText(0, top, lineNumberArea->width() - 3, fontMetrics().height(),
                           Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + blockBoundingGeometry(block).height();
        ++blockNumber;
    }
}

void CodeEditor::setErrors(const std::vector<EditorError>& newErrors)
{
    errors = newErrors;
    highlightErrorLine();
}

void CodeEditor::clearErrors()
{
    errors.clear();
    setExtraSelections({});
}

void CodeEditor::highlightErrorLine()
{
    // Create extra selections for error underlining
    QList<QTextEdit::ExtraSelection> extraSelections;

    for (const auto& error : errors) {
        QTextBlock block = document()->findBlockByNumber(error.line - 1);
        if (!block.isValid()) {
            continue;
        }

        // Underline specific error position and keep selection inside the same block.
        QTextEdit::ExtraSelection underline;
        underline.format.setUnderlineColor(QColor("#FF4C4C"));
        underline.format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

        const int lineLen = block.text().length();
        if (lineLen <= 0) {
            QTextEdit::ExtraSelection lineSelection;
            lineSelection.format.setBackground(QColor(255, 0, 0, 26));
            lineSelection.format.setProperty(QTextFormat::FullWidthSelection, true);
            lineSelection.cursor = QTextCursor(block);
            extraSelections.append(lineSelection);
            continue;
        }

        const int startCol = std::max(1, std::min(error.column, lineLen));
        const int endCol = std::max(startCol, std::min(error.endColumn, lineLen));

        QTextCursor cur = QTextCursor(block);
        cur.setPosition(block.position() + (startCol - 1));
        const int length = std::max(1, (endCol - startCol) + 1);
        cur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, length);
        underline.cursor = cur;
        extraSelections.append(underline);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::setExtraSelectionsMerged(const QList<QTextEdit::ExtraSelection>& customSelections)
{
    QList<QTextEdit::ExtraSelection> mergedSelections = customSelections;

    for (const auto& error : errors) {
        QTextBlock block = document()->findBlockByNumber(error.line - 1);
        if (!block.isValid()) {
            continue;
        }

        QTextEdit::ExtraSelection underline;
        underline.format.setUnderlineColor(QColor("#E05C6A"));
        underline.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);

        const int lineLen = block.text().length();
        if (lineLen <= 0) {
            QTextEdit::ExtraSelection lineSelection;
            lineSelection.format.setBackground(QColor(255, 0, 0, 20));
            lineSelection.format.setProperty(QTextFormat::FullWidthSelection, true);
            lineSelection.cursor = QTextCursor(block);
            mergedSelections.append(lineSelection);
            continue;
        }

        const int startCol = std::max(1, std::min(error.column, lineLen));
        const int endCol = std::max(startCol, std::min(error.endColumn, lineLen));
        QTextCursor cur(block);
        const int start = startCol - 1;
        const int len = std::max(1, (endCol - startCol) + 1);
        cur.setPosition(block.position() + start);
        cur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, len);
        underline.cursor = cur;
        mergedSelections.append(underline);
    }

    setExtraSelections(mergedSelections);
}

void CodeEditor::mouseMoveEvent(QMouseEvent* event)
{
    QPlainTextEdit::mouseMoveEvent(event);

    // Check if hovering over an error
    QTextCursor cursor = cursorForPosition(event->pos());
    int blockNumber = cursor.blockNumber() + 1;

    hoveredErrorIndex = -1;
    for (int i = 0; i < errors.size(); ++i) {
        if (errors[i].line == blockNumber) {
            hoveredErrorIndex = i;
            errorTooltip = QString::fromStdString(errors[i].message);
            QToolTip::showText(event->globalPos(), errorTooltip, this);
            break;
        }
    }

    if (hoveredErrorIndex == -1) {
        QToolTip::hideText();
    }
}

void CodeEditor::leaveEvent(QEvent* event)
{
    QPlainTextEdit::leaveEvent(event);
    QToolTip::hideText();
    hoveredErrorIndex = -1;
}

void CodeEditor::paintEvent(QPaintEvent* event)
{
    // Draw line highlight for current line
    QPlainTextEdit::paintEvent(event);
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent* event)
{
    codeEditor->lineNumberAreaPaintEvent(event);
}

// ============ NEW: Auto-Complete & Smart Features ============

const QStringList CodeEditor::keywordSuggestions = {
    "switch", "case", "default", "break",
    "int", "string", "void",
    "using", "namespace", "std", "#include",
    "return", "cin", "cout"
};

void CodeEditor::setupAutoComplete()
{
    completeModel = new QStringListModel(keywordSuggestions, this);
    autoCompleter = new QCompleter(completeModel, this);
    autoCompleter->setCompletionMode(QCompleter::PopupCompletion);
    autoCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    autoCompleter->setWidget(this);
    
    connect(autoCompleter, QOverload<const QString&>::of(&QCompleter::activated),
            this, &CodeEditor::onAutoCompleteActivated);
}

void CodeEditor::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Tab) {
        // Replace tab with spaces
        insertPlainText("    ");
        return;
    }
    
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        // Smart indent on new line
        QPlainTextEdit::keyPressEvent(event);
        smartIndent();
        return;
    }
    
    if (event->key() == Qt::Key_Z && event->modifiers() & Qt::ControlModifier) {
        // Ctrl+Z: Undo
        undo();
        return;
    }
    
    if (event->key() == Qt::Key_Y && event->modifiers() & Qt::ControlModifier) {
        // Ctrl+Y: Redo
        redo();
        return;
    }
    
    if (event->key() == Qt::Key_Space && event->modifiers() & Qt::ControlModifier) {
        // Ctrl+Space: Show auto-complete
        if (!autoCompleter) setupAutoComplete();
        
        QTextCursor cursor = textCursor();
        QTextBlock block = cursor.block();
        int posInBlock = cursor.positionInBlock();
        
        // Find start of current word
        int start = posInBlock;
        while (start > 0 && (block.text()[start - 1].isLetterOrNumber() || block.text()[start - 1] == '_')) {
            start--;
        }
        
        QString prefix = block.text().mid(start, posInBlock - start);
        autoCompleter->setCompletionPrefix(prefix);
        
        QRect cursorRect = this->cursorRect();
        cursorRect.setWidth(150);
        autoCompleter->complete(cursorRect);
        return;
    }
    
    // Check for opening bracket to insert matching closing bracket
    if (!event->text().isEmpty() && (event->text()[0] == '{' || event->text()[0] == '(' || event->text()[0] == '[')) {
        QPlainTextEdit::keyPressEvent(event);
        insertMatchingBracket(event->text()[0]);
        return;
    }
    
    QPlainTextEdit::keyPressEvent(event);
    highlightMatchingBrackets();
}

void CodeEditor::insertMatchingBracket(QChar openBracket)
{
    QChar closeBracket;
    if (openBracket == '{') closeBracket = '}';
    else if (openBracket == '(') closeBracket = ')';
    else if (openBracket == '[') closeBracket = ']';
    else return;
    
    QTextCursor cursor = textCursor();
    cursor.insertText(QString(closeBracket));
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
}

void CodeEditor::smartIndent()
{
    QTextCursor cursor = textCursor();
    QTextBlock prevBlock = cursor.block().previous();
    
    if (!prevBlock.isValid()) return;
    
    QString prevText = prevBlock.text();
    int indent = 0;
    for (QChar ch : prevText) {
        if (ch == ' ') indent++;
        else if (ch == '\t') indent += 4;
        else break;
    }
    
    // Check if previous line ends with { or :
    if (prevText.trimmed().endsWith("{") || prevText.trimmed().endsWith(":")) {
        indent += 4;
    }
    
    cursor.insertText(QString(indent, ' '));
    setTextCursor(cursor);
}

void CodeEditor::highlightMatchingBrackets()
{
    QTextCursor cursor = textCursor();
    int startPos = -1, endPos = -1;
    
    if (findMatchingBracket(startPos, endPos)) {
        QList<QTextEdit::ExtraSelection> selections;
        
        QTextEdit::ExtraSelection sel1, sel2;
        QTextCharFormat fmt;
        fmt.setBackground(QColor(70, 84, 128));  // Subtle highlight
        
        sel1.format = fmt;
        sel1.cursor = cursor;
        sel1.cursor.setPosition(startPos);
        sel1.cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        selections.append(sel1);
        
        sel2.format = fmt;
        sel2.cursor = cursor;
        sel2.cursor.setPosition(endPos);
        sel2.cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        selections.append(sel2);
        
        setExtraSelectionsMerged(selections);
    } else {
        setExtraSelectionsMerged({});
    }
}

bool CodeEditor::findMatchingBracket(int& startPos, int& endPos)
{
    QTextCursor cursor = textCursor();
    int pos = cursor.position() - 1;
    
    QString txt = toPlainText();
    if (pos < 0 || pos >= txt.length()) return false;
    
    QChar ch = txt[pos];
    QChar matching;
    int direction = 1;
    
    if (ch == '{') matching = '}';
    else if (ch == '}') { matching = '{'; direction = -1; }
    else if (ch == '(') matching = ')';
    else if (ch == ')') { matching = '('; direction = -1; }
    else if (ch == '[') matching = ']';
    else if (ch == ']') { matching = '['; direction = -1; }
    else return false;
    
    int count = 1;
    int searchPos = pos + direction;
    
    while (searchPos >= 0 && searchPos < txt.length()) {
        QChar current = txt[searchPos];
        if (current == ch) count++;
        else if (current == matching) count--;
        
        if (count == 0) {
            startPos = direction > 0 ? pos : searchPos;
            endPos = direction > 0 ? searchPos : pos;
            return true;
        }
        
        searchPos += direction;
    }
    
    return false;
}

void CodeEditor::onAutoCompleteActivated(const QString& text)
{
    QTextCursor cursor = textCursor();
    QTextBlock block = cursor.block();
    int posInBlock = cursor.positionInBlock();
    
    // Find start of current word
    int start = posInBlock;
    while (start > 0 && (block.text()[start - 1].isLetterOrNumber() || block.text()[start - 1] == '_')) {
        start--;
    }
    
    // Replace word with completion
    cursor.setPosition(block.position() + start);
    cursor.setPosition(block.position() + posInBlock, QTextCursor::KeepAnchor);
    cursor.insertText(text);
    setTextCursor(cursor);
}

void CodeEditor::updateAutoCompleteModel()
{
    QStringList allWords = keywordSuggestions;
    QRegularExpression re("\\b[a-zA-Z_][a-zA-Z0-9_]*\\b");
    QString text = toPlainText();
    QRegularExpressionMatchIterator i = re.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString word = match.captured(0);
        if (!allWords.contains(word)) {
            allWords.append(word);
        }
    }
    completeModel->setStringList(allWords);
}
