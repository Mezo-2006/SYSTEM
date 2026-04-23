#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QTimer>
#include <QSplitter>
#include <QFrame>
#include <map>
#include <vector>
#include <string>
#include "../core/TACGenerator.h"

class TACSimulator : public QWidget {
    Q_OBJECT
public:
    explicit TACSimulator(QWidget* parent = nullptr);
    void loadProgram(const std::vector<TACInstruction>& instructions);
    void reset();

private slots:
    void stepForward();
    void stepBack();
    void autoPlay();
    void onSpeedChanged(int v);
    void tick();

private:
    // ── data ──────────────────────────────────────────────────────────────
    std::vector<TACInstruction> program;
    int  pc = 0;          // program counter
    bool running = false;
    std::map<std::string, std::string> vars;   // variable/temp → value
    std::vector<std::map<std::string, std::string>> history; // snapshot per step
    std::vector<int> pcHistory;

    // ── helpers ───────────────────────────────────────────────────────────
    void executeStep();
    void refreshUI();
    void highlightPC();
    std::string resolve(const std::string& s) const;
    int  resolveInt(const std::string& s) const;
    void logEvent(const QString& msg, const QColor& col = QColor("#94A3B8"));

    // ── widgets ───────────────────────────────────────────────────────────
    QTableWidget* tacTable;
    QTableWidget* varTable;
    QTextEdit*    logView;
    QPushButton*  btnBack;
    QPushButton*  btnStep;
    QPushButton*  btnPlay;
    QPushButton*  btnReset;
    QLabel*       pcLabel;
    QLabel*       statusLabel;
    QSlider*      speedSlider;
    QTimer*       timer;
};
