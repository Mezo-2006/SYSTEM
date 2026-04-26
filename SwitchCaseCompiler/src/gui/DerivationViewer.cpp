#include "DerivationViewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QSignalBlocker>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

DerivationViewer::DerivationViewer(QWidget* parent) 
    : QWidget(parent), currentStep(0), isPlaying(false) {
    setupUI();
    
    autoPlayTimer = new QTimer(this);
    connect(autoPlayTimer, &QTimer::timeout, this, &DerivationViewer::onAutoPlayTick);
}

void DerivationViewer::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Step counter
    stepCounterLabel = new QLabel("Step: 0 / 0");
    stepCounterLabel->setStyleSheet(
        "QLabel {"
        "  font-size: 14px;"
        "  font-weight: 700;"
        "  color: #cccccc;"
        "  background-color: #252526;"
        "  border: 1px solid #3c3c3c;"
        "  border-radius: 4px;"
        "  padding: 6px 10px;"
        "}"
    );
    layout->addWidget(stepCounterLabel);

    stepByStepModeCheck = new QCheckBox("Manual step mode");
    stepByStepModeCheck->setChecked(false);
    layout->addWidget(stepByStepModeCheck);

    QHBoxLayout* mainLayout = new QHBoxLayout();

    // Left: step list
    QVBoxLayout* listLayout = new QVBoxLayout();
    listLayout->addWidget(new QLabel("Derivation Step List"));
    stepListWidget = new QListWidget();
    stepListWidget->setMinimumWidth(320);
    stepListWidget->setStyleSheet(
        "QListWidget {"
        "  background-color: #1e1e1e;"
        "  color: #d4d4d4;"
        "  border: 1px solid #3c3c3c;"
        "  border-radius: 4px;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #094771;"
        "  color: #ffffff;"
        "}"
    );
    listLayout->addWidget(stepListWidget);

    QWidget* listWidget = new QWidget();
    listWidget->setLayout(listLayout);

    // Right: current step details
    QVBoxLayout* detailsLayout = new QVBoxLayout();

    // Sentential form display
    sententialFormDisplay = new QTextEdit();
    sententialFormDisplay->setReadOnly(true);
    sententialFormDisplay->setMinimumHeight(150);
    sententialFormDisplay->setStyleSheet(
        "QTextEdit {"
        "  background-color: #1e1e1e;"
        "  color: #d4d4d4;"
        "  border: 1px solid #3c3c3c;"
        "  border-radius: 4px;"
        "  padding: 15px;"
        "}"
    );
    detailsLayout->addWidget(sententialFormDisplay);

    // Production rule
    productionRuleLabel = new QLabel("➤ Production Applied: ");
    productionRuleLabel->setStyleSheet(
        "QLabel {"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  padding: 10px 14px;"
        "  background-color: #252526;"
        "  color: #ffffff;"
        "  border-radius: 4px;"
        "  border: 1px solid #3c3c3c;"
        "  margin-top: 5px;"
        "}"
    );
    detailsLayout->addWidget(productionRuleLabel);

    rightmostNonTerminalLabel = new QLabel("🎯 Target rightmost non-terminal: (none)");
    rightmostNonTerminalLabel->setStyleSheet(
        "QLabel {"
        "  font-size: 13px; "
        "  padding: 8px 12px; "
        "  background-color: #4d0000; "
        "  border-left: 4px solid #f48771; "
        "  border-radius: 4px; "
        "  color: #ffcccc; "
        "  font-weight: 600; "
        "  margin-bottom: 5px;"
        "}"
    );
    detailsLayout->addWidget(rightmostNonTerminalLabel);

    // Controls
    QHBoxLayout* controlLayout = new QHBoxLayout();

    jumpToStartButton = new QPushButton("|◀ Start");
    stepBackButton = new QPushButton("◀ Step Back");
    stepForwardButton = new QPushButton("Step Forward ▶");
    autoPlayButton = new QPushButton("▶ Auto Play");
    jumpToEndButton = new QPushButton("End ▶|");
    exportRulesButton = new QPushButton("💾 Export Rules");

    controlLayout->addWidget(jumpToStartButton);
    controlLayout->addWidget(stepBackButton);
    controlLayout->addWidget(stepForwardButton);
    controlLayout->addWidget(autoPlayButton);
    controlLayout->addWidget(jumpToEndButton);
    controlLayout->addWidget(exportRulesButton);

    detailsLayout->addLayout(controlLayout);

    // Speed control
    QHBoxLayout* speedLayout = new QHBoxLayout();
    speedLayout->addWidget(new QLabel("Speed:"));
    speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setMinimum(100);
    speedSlider->setMaximum(2000);
    speedSlider->setValue(1000);
    speedLayout->addWidget(speedSlider);
    detailsLayout->addLayout(speedLayout);

    QHBoxLayout* scrubberLayout = new QHBoxLayout();
    scrubberLayout->addWidget(new QLabel("Step timeline:"));
    stepScrubber = new QSlider(Qt::Horizontal);
    stepScrubber->setMinimum(0);
    stepScrubber->setMaximum(0);
    stepScrubber->setValue(0);
    scrubberLayout->addWidget(stepScrubber);
    detailsLayout->addLayout(scrubberLayout);

    QWidget* detailsWidget = new QWidget();
    detailsWidget->setLayout(detailsLayout);

    mainLayout->addWidget(listWidget);
    mainLayout->addWidget(detailsWidget, 1);
    layout->addLayout(mainLayout);

    // Connect signals
    connect(stepBackButton, &QPushButton::clicked, this, &DerivationViewer::onStepBack);
    connect(stepForwardButton, &QPushButton::clicked, this, &DerivationViewer::onStepForward);
    connect(autoPlayButton, &QPushButton::clicked, this, &DerivationViewer::onAutoPlay);
    connect(speedSlider, &QSlider::valueChanged, this, &DerivationViewer::onSpeedChanged);
    connect(stepListWidget, &QListWidget::currentRowChanged, this, &DerivationViewer::onStepSelected);
    connect(stepByStepModeCheck, &QCheckBox::toggled, this, &DerivationViewer::onStepModeToggled);
    connect(stepScrubber, &QSlider::valueChanged, this, &DerivationViewer::onStepScrubberMoved);
    connect(exportRulesButton, &QPushButton::clicked, this, &DerivationViewer::onExportRules);

    connect(jumpToStartButton, &QPushButton::clicked, this, [this]() {
        goToStep(0);
    });
    connect(jumpToEndButton, &QPushButton::clicked, this, [this]() {
        if (!steps.empty()) {
            goToStep(static_cast<int>(steps.size()) - 1);
        }
    });

    onStepModeToggled(stepByStepModeCheck->isChecked());
    updateControlStates();
}

void DerivationViewer::setDerivationSteps(const std::vector<DerivationStep>& derivationSteps) {
    steps = derivationSteps;
    currentStep = 0;

    refreshStepList();

    if (!steps.empty()) {
        displayStep(0);
        if (stepListWidget) {
            stepListWidget->setCurrentRow(0);
        }
    } else {
        sententialFormDisplay->clear();
        productionRuleLabel->setText("➤ Production Applied: ");
        rightmostNonTerminalLabel->setText("🎯 Target rightmost non-terminal: (none)");
    }

    {
        QSignalBlocker blocker(stepScrubber);
        stepScrubber->setMaximum(steps.empty() ? 0 : static_cast<int>(steps.size()) - 1);
        stepScrubber->setValue(0);
    }

    stepCounterLabel->setText(QString("Step: %1 / %2").arg(steps.empty() ? 0 : (currentStep + 1)).arg(steps.size()));
    updateControlStates();
}

void DerivationViewer::refreshStepList() {
    stepListWidget->clear();
    for (size_t i = 0; i < steps.size(); ++i) {
        const auto& step = steps[i];
        QString rule = QString::fromStdString(step.productionRule);
        if (rule.length() > 34) {
            rule = rule.left(31) + "...";
        }

        QString preview = QString::fromStdString(step.sententialForm);
        if (preview.length() > 30) {
            preview = preview.left(27) + "...";
        }

        stepListWidget->addItem(QString("%1 | %2 | %3")
            .arg(i + 1, 3, 10, QChar('0'))
            .arg(rule)
            .arg(preview));
    }
}

void DerivationViewer::goToStep(int stepIndex) {
    if (steps.empty()) {
        return;
    }
    if (stepIndex < 0 || stepIndex >= static_cast<int>(steps.size())) {
        return;
    }

    currentStep = stepIndex;
    displayStep(currentStep);
    if (stepListWidget && stepListWidget->currentRow() != currentStep) {
        QSignalBlocker blocker(stepListWidget);
        stepListWidget->setCurrentRow(currentStep);
    }
    updateControlStates();
}

void DerivationViewer::displayStep(int stepIndex) {
    if (stepIndex < 0 || stepIndex >= static_cast<int>(steps.size())) {
        return;
    }
    
    const auto& step = steps[stepIndex];
    
    // Display sentential form with rich formatting (HTML chips mapping)
    sententialFormDisplay->clear();
    
    QString text = QString::fromStdString(step.sententialForm);
    
    auto styleTokens = [](const QString& str) -> QString {
        if (str.isEmpty()) return "";
        QStringList tokens = str.split(" ", Qt::SkipEmptyParts);
        QString html;
        for (const QString& t : tokens) {
            bool isTerminal = (t == "switch" || t == "case" || t == "break" || t == "default" ||
                               t == "{" || t == "}" || t == "(" || t == ")" || 
                               t == ":" || t == ";" || t == "=" || t == "+" || t == "-" || 
                               t == "*" || t == "/" || t == "<" || t == ">" || t == "cin" || 
                               t == "cout" || t == "int" || t == "float" || t == "double" ||
                               t == "bool" || t == "char" || t == "id" || t == "num");
            
            if (isTerminal) {
                html += QString("<span style='background-color: #252526; color: #4ec9b0;'>&nbsp;%1&nbsp;</span> ").arg(t.toHtmlEscaped());
            } else {
                html += QString("<span style='background-color: #1e1e1e; color: #569cd6;'>&nbsp;%1&nbsp;</span> ").arg(t.toHtmlEscaped());
            }
        }
        return html;
    };

    QString html = "<div style='font-family: \"Consolas\", monospace; font-size: 16px; line-height: 2.2; text-align: center;'>";
    
    if (step.rightmostNonTerminalPos >= 0 && step.rightmostNonTerminalPos <= text.length()) {
        QString before = text.left(step.rightmostNonTerminalPos);
        QString target = QString::fromStdString(step.rightmostNonTerminal);
        QString after = text.mid(step.rightmostNonTerminalPos + target.length());
        
        html += styleTokens(before);
        if (!target.isEmpty()) {
            html += QString("<span style='background-color: #4d0000; color: #f48771; border: 2px solid #f48771; font-weight: bold; font-size: 18px;'>&nbsp;%1&nbsp;</span> ").arg(target.toHtmlEscaped());
        }
        html += styleTokens(after);
    } else {
        html += styleTokens(text);
    }
    
    html += "</div>";
    sententialFormDisplay->setHtml(html);
    
    // Display production rule
    productionRuleLabel->setText(QString("➤ Production Applied: %1").arg(
        QString::fromStdString(step.productionRule)));

    if (step.rightmostNonTerminal.empty() || step.rightmostNonTerminalPos < 0) {
        rightmostNonTerminalLabel->setText("🎯 Target rightmost non-terminal: (none)");
    } else {
        rightmostNonTerminalLabel->setText(
            QString("🎯 Target rightmost non-terminal: %1 (position %2)")
                .arg(QString::fromStdString(step.rightmostNonTerminal))
                .arg(step.rightmostNonTerminalPos)
        );
    }
    
    // Update step counter
    stepCounterLabel->setText(QString("Step: %1 / %2").arg(stepIndex + 1).arg(steps.size()));

    if (stepListWidget && stepListWidget->currentRow() != stepIndex) {
        QSignalBlocker blocker(stepListWidget);
        stepListWidget->setCurrentRow(stepIndex);
    }

    if (stepScrubber && stepScrubber->value() != stepIndex) {
        QSignalBlocker blocker(stepScrubber);
        stepScrubber->setValue(stepIndex);
    }
}

void DerivationViewer::highlightRightmostNonTerminal(const std::string& text, int pos, 
                                                     const std::string& nonTerm) {
    // Implemented in displayStep
}

void DerivationViewer::onStepBack() {
    if (currentStep > 0) {
        goToStep(currentStep - 1);
    }
}

void DerivationViewer::onStepForward() {
    if (currentStep < static_cast<int>(steps.size()) - 1) {
        goToStep(currentStep + 1);
    }
}

void DerivationViewer::onAutoPlay() {
    if (steps.empty()) {
        return;
    }

    if (stepByStepModeCheck->isChecked()) {
        stepByStepModeCheck->setChecked(false);
    }

    if (!isPlaying && currentStep >= static_cast<int>(steps.size()) - 1) {
        goToStep(0);
    }

    isPlaying = !isPlaying;
    
    if (isPlaying) {
        autoPlayButton->setText("⏸ Pause");
        autoPlayTimer->start(speedSlider->value());
    } else {
        autoPlayButton->setText("▶ Auto Play");
        autoPlayTimer->stop();
    }
}

void DerivationViewer::onAutoPlayTick() {
    if (currentStep < static_cast<int>(steps.size()) - 1) {
        goToStep(currentStep + 1);
    } else {
        isPlaying = false;
        autoPlayButton->setText("▶ Auto Play");
        autoPlayTimer->stop();
    }
}

void DerivationViewer::onSpeedChanged(int value) {
    if (isPlaying) {
        autoPlayTimer->setInterval(value);
    }
}

void DerivationViewer::onStepSelected(int row) {
    if (row >= 0 && row < static_cast<int>(steps.size()) && row != currentStep) {
        currentStep = row;
        displayStep(currentStep);
    }
}

void DerivationViewer::onStepModeToggled(bool enabled) {
    if (enabled && isPlaying) {
        isPlaying = false;
        autoPlayTimer->stop();
        autoPlayButton->setText("▶ Auto Play");
    }
    if (enabled) {
        autoPlayButton->setToolTip("Manual mode is enabled. Uncheck manual mode or press Auto Play to switch automatically.");
    } else {
        autoPlayButton->setToolTip("Automatically animate derivation steps.");
    }
}

void DerivationViewer::onStepScrubberMoved(int value) {
    if (!steps.empty() && value != currentStep) {
        goToStep(value);
    }
}

void DerivationViewer::updateControlStates() {
    const bool hasSteps = !steps.empty();
    jumpToStartButton->setEnabled(hasSteps && currentStep > 0);
    stepBackButton->setEnabled(hasSteps && currentStep > 0);
    stepForwardButton->setEnabled(hasSteps && currentStep < static_cast<int>(steps.size()) - 1);
    jumpToEndButton->setEnabled(hasSteps && currentStep < static_cast<int>(steps.size()) - 1);
    autoPlayButton->setEnabled(hasSteps);
    stepListWidget->setEnabled(hasSteps);
    stepScrubber->setEnabled(hasSteps);
    exportRulesButton->setEnabled(hasSteps);
}

void DerivationViewer::onExportRules() {
    if (steps.empty()) return;
    
    QString fileName = QFileDialog::getSaveFileName(this, "Export Derivation Rules", "", "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Export Error", "Could not open file for writing:\n" + file.errorString());
        return;
    }
    
    QTextStream out(&file);
    out << "Rightmost Derivation Rules Applied:\n";
    out << "=================================\n\n";
    
    for (size_t i = 0; i < steps.size(); ++i) {
        if (!steps[i].productionRule.empty()) {
            out << QString::number(i + 1) << ". " << QString::fromStdString(steps[i].productionRule) << "\n";
        }
    }
    
    file.close();
    QMessageBox::information(this, "Export Successful", "The derivation rules have been exported successfully to:\n" + fileName);
}
