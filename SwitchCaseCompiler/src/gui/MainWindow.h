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
class ASTView;
class ThreeColumnView;
class OptimizationPanel;
class SyntaxHighlighter;
class IRFlowDiagram;
class PerformanceMetrics;
class TokenChart;
class PipelineDiagram;
class LifetimeDiagram;

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
    std::array<QLabel*, 6> phaseBadges{};
    std::array<QLabel*, 6> phaseDurations{};
    
    // Tab 2: Derivation Viewer
    QWidget* derivationTab;
    DerivationViewer* derivationViewer;
    
    // Tab 3: Parse Tree
    QWidget* parseTreeTab;
    ParseTreeView* parseTreeView;

    // Tab 4: AST View
    QWidget* astTab;
    ASTView* astView;

    // Tab 5: CFG
    QWidget* cfgTab;
    QTextEdit* cfgTextView;
    
    // Tab 6: Token Table
    QWidget* tokenTab;
    QLabel* tokenCountLabel;
    QTableWidget* tokenTable;
    
    // Tab 7: Symbol Table
    QWidget* symbolTab;
    QTableWidget* symbolTable;
    
    // Tab 8: Three Column View (Source/TAC/Assembly)
    QWidget* traceTab;
    ThreeColumnView* threeColumnView;
    
    // Tab 9: Optimization Panel
    QWidget* optimizationTab;
    OptimizationPanel* optimizationPanel;
    
    // Tab 10: Error Panel
    QWidget* errorTab;
    QTableWidget* errorTable;
    
    // Tab 11: Output Console
    QWidget* consoleTab;
    QTextEdit* consoleOutput;

    // Tab 12: Token Insights
    QWidget* tokenInsightsTab;
    TokenChart* tokenChart;

    // Tab 13: Pipeline Studio
    QWidget* pipelineTab;
    PipelineDiagram* pipelineDiagram;

    // Tab 14: Lifetime View
    QWidget* lifetimeTab;
    LifetimeDiagram* lifetimeDiagram;
    
    // Legacy Tab: IR Flow Diagram (kept in code, not shown in UI)
    QWidget* irFlowTab;
    IRFlowDiagram* irFlowDiagram;
    
    // Tab 15: Performance Metrics
    QWidget* performanceTab;
    PerformanceMetrics* performanceMetrics;
    
    void setupUI();  // Main UI setup
    void setupEditorTab();
    void setupDerivationTab();
    void setupParseTreeTab();
    void setupASTTab();
    void setupCFGTab();
    void setupTokenTab();
    void setupSymbolTab();
    void setupTraceTab();
    void setupOptimizationTab();
    void setupErrorTab();
    void setupConsoleTab();
    void setupTokenInsightsTab();
    void setupPipelineTab();
    void setupLifetimeTab();
    void setupIRFlowTab();
    void setupPerformanceTab();
    
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
