#ifndef DERIVATION_VIEWER_H
#define DERIVATION_VIEWER_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QTimer>
#include <QListWidget>
#include <QCheckBox>
#include "../core/Parser.h"

class DerivationViewer : public QWidget {
    Q_OBJECT
    
private:
    std::vector<DerivationStep> steps;
    int currentStep;
    
    QTextEdit* sententialFormDisplay;
    QLabel* productionRuleLabel;
    QLabel* rightmostNonTerminalLabel;
    QLabel* stepCounterLabel;
    QListWidget* stepListWidget;
    QCheckBox* stepByStepModeCheck;
    
    QPushButton* stepBackButton;
    QPushButton* stepForwardButton;
    QPushButton* autoPlayButton;
    QPushButton* jumpToStartButton;
    QPushButton* jumpToEndButton;
    QPushButton* exportRulesButton;
    QSlider* stepScrubber;
    QSlider* speedSlider;
    
    QTimer* autoPlayTimer;
    bool isPlaying;
    
    void setupUI();
    void displayStep(int stepIndex);
    void highlightRightmostNonTerminal(const std::string& text, int pos, const std::string& nonTerm);
    void refreshStepList();
    void updateControlStates();
    
public:
    explicit DerivationViewer(QWidget* parent = nullptr);
    
    void setDerivationSteps(const std::vector<DerivationStep>& derivationSteps);
    void goToStep(int stepIndex);
    
private slots:
    void onStepBack();
    void onStepForward();
    void onAutoPlay();
    void onAutoPlayTick();
    void onSpeedChanged(int value);
    void onStepSelected(int row);
    void onStepModeToggled(bool enabled);
    void onStepScrubberMoved(int value);
    void onExportRules();
};

#endif // DERIVATION_VIEWER_H
