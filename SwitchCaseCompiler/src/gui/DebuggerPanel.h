#ifndef DEBUGGER_PANEL_H
#define DEBUGGER_PANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTextEdit>
#include <QTabWidget>
#include <memory>
#include <vector>
#include <map>

class DebuggerPanel : public QWidget {
    Q_OBJECT

public:
    explicit DebuggerPanel(QWidget* parent = nullptr);
    ~DebuggerPanel();
    
    // Update debugger with derivation steps
    void setDerivationSteps(const std::vector<std::pair<std::string, std::string>>& steps);
    
    // Update parse tree statistics
    void updateStatistics(int treeDepth, int totalNodes, int terminals, int nonTerminals);
    
    // Update variable inspector
    void updateVariables(const std::map<std::string, std::pair<std::string, std::string>>& vars);
    
    // Get current step index
    int getCurrentStepIndex() const { return currentStepIndex; }
    
    // Get current sentential form
    QString getCurrentSententialForm() const;

signals:
    void stepChanged(int newIndex);

private slots:
    void onPlayPause();
    void onReverse();
    void onStepForward();
    void onStepBackward();
    void onSliderMoved(int value);
    void onSpeedChanged(int value);
    void updateCurrentStep();
    void onRuleDoubleClicked(QTreeWidgetItem* item, int column);

private:
    void setupUI();
    void setupControlPanel();
    void setupStatisticsPanel();
    void setupDebuggerPanel();
    void setupVariableInspector();
    void setupRuleExplorer();
    void updateStepDisplay();
    
    // UI Components
    QTabWidget* tabWidget;
    
    // Control Panel
    QPushButton* playPauseBtn;
    QPushButton* reverseBtn;
    QPushButton* stepFwdBtn;
    QPushButton* stepBwdBtn;
    QSlider* stepSlider;
    QSpinBox* speedSpinBox;
    QLabel* stepCountLabel;
    
    // Statistics Panel
    QLabel* treeDepthLabel;
    QLabel* nodeCountLabel;
    QLabel* terminalCountLabel;
    QLabel* nonTerminalCountLabel;
    QLabel* depthPercentLabel;
    QTextEdit* frequencyLabel;  // FIXED: Should be QTextEdit
    
    // Derivation Viewer
    QTextEdit* sententialFormDisplay;
    QTextEdit* derivationStepsDisplay;
    QLabel* currentRuleLabel;
    QLabel* expandedNodeLabel;
    
    // Variable Inspector
    QTableWidget* variableTable;
    
    // Rule Explorer
    QTreeWidget* ruleTree;
    QTextEdit* ruleDetailsDisplay;
    
    // Data
    std::vector<std::pair<std::string, std::string>> derivationSteps;  // (sentential form, rule)
    std::map<std::string, std::pair<std::string, std::string>> variables;  // name -> (type, value)
    
    int currentStepIndex;
    bool isPlaying;
    int playbackSpeed;  // ms per step
    
    int treeDepth;
    int totalNodes;
    int terminalCount;
    int nonTerminalCount;
    
    QTimer* playbackTimer;
};

#endif // DEBUGGER_PANEL_H
