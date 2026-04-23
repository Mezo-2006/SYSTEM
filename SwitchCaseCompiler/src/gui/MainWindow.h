#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QProgressBar>
#include <memory>
#include <array>

// Forward declarations
class DerivationViewer;
class ParseTreeView;
class ThreeColumnView;
class OptimizationPanel;
class SyntaxHighlighter;

// Core compiler components
#include "../core/Lexer.h"
#include "../core/Parser.h"
#include "../core/SemanticAnalyzer.h"
#include "../core/TACGenerator.h"
#include "../core/Optimizer.h"
#include "../core/CodeGenerator.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
    
private:
    // Compiler components
    std::unique_ptr<Lexer> lexer;
    std::unique_ptr<Parser> parser;
    std::unique_ptr<SemanticAnalyzer> semanticAnalyzer;
    std::unique_ptr<TACGenerator> tacGenerator;
    std::unique_ptr<Optimizer> optimizer;
    std::unique_ptr<CodeGenerator> codeGenerator;
    std::unique_ptr<TargetCodeOptimizer> targetOptimizer;
    
    // Syntax highlighter
    SyntaxHighlighter* syntaxHighlighter;
    
    // GUI Components
    QTabWidget* tabWidget;
    
    // Tab 1: Source Editor
    QWidget* editorTab;
    class CodeEditor* sourceEditor;  // Professional code editor with line numbers + error underlining
    QTableWidget* editorErrorTable;  // Error display in editor tab
    QPushButton* compileButton;
    QLabel* statusLabel;
    QLabel* validityLabel;
    QProgressBar* phaseProgressBar;
    QFrame* phaseTimelineFrame;
    std::array<QLabel*, 7> phaseBadges{};
    std::array<QLabel*, 7> phaseDurations{};
    
    // Tab 2: Derivation Viewer
    QWidget* derivationTab;
    DerivationViewer* derivationViewer;
    
    // Tab 3: Parse Tree
    QWidget* parseTreeTab;
    ParseTreeView* parseTreeView;

    // Tab 4: CFG
    QWidget* cfgTab;
    QTextEdit* cfgTextView;
    
    // Tab 5: Token Table
    QWidget* tokenTab;
    QTableWidget* tokenTable;
    
    // Tab 6: Symbol Table
    QWidget* symbolTab;
    QTableWidget* symbolTable;
    
    // Tab 7: Three Column View (Source/TAC/Assembly)
    QWidget* traceTab;
    ThreeColumnView* threeColumnView;
    
    // Tab 8: Optimization Panel
    QWidget* optimizationTab;
    OptimizationPanel* optimizationPanel;
    
    // Tab 9: Error Panel
    QWidget* errorTab;
    QTableWidget* errorTable;
    
    // Tab 10: Output Console
    QWidget* consoleTab;
    QTextEdit* consoleOutput;
    
    void setupUI();  // Main UI setup
    void setupEditorTab();
    void setupDerivationTab();
    void setupParseTreeTab();
    void setupCFGTab();
    void setupTokenTab();
    void setupSymbolTab();
    void setupTraceTab();
    void setupOptimizationTab();
    void setupErrorTab();
    void setupConsoleTab();
    
    void updateTokenTable();
    void updateSymbolTable();
    void updateErrorTable();
    void logToConsole(const std::string& message);
    
    void highlightError(int line, int column);
    void connectSignals();  // Connect all signals/slots
    
    // Parse tree interactivity
    void onParseTreeNodeHighlighted(int line, int column, int endLine, int endColumn, const QString& tooltip);
    void onParseTreeNodeClicked(int derivationStep);
    void onParseTreeLexemeRequested(const QString& lexeme, const QString& tooltip);

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
private:
    QTimer* parseTimer;  // Debounce timer for real-time parsing
    
private slots:
    void onCompile();
    void onErrorClicked(int row, int column);
    void onSourceCodeChanged();         // Real-time parsing - moved here
    void performRealTimeParse();        // Real-time parsing - moved here
};

#endif // MAINWINDOW_H
