#pragma once
#include <QDialog>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <vector>
#include <string>
#include "../core/TACGenerator.h"
#include "../core/Lexer.h"
#include "../core/Optimizer.h"

struct CompileStats {
    int   tokenCount      = 0;
    int   derivationSteps = 0;
    int   astNodes        = 0;
    int   tacBefore       = 0;
    int   tacAfter        = 0;
    int   assemblyLines   = 0;
    int   symbolCount     = 0;
    int   caseCount       = 0;
    int   sourceLines     = 0;
    qint64 totalMs        = 0;
    std::vector<qint64> phaseMs;   // 7 phases
    std::vector<std::string> phaseNames;
    bool  success         = true;
};

class CompileReport : public QDialog {
    Q_OBJECT
    Q_PROPERTY(qreal revealProgress READ revealProgress WRITE setRevealProgress)
public:
    explicit CompileReport(QWidget* parent = nullptr);
    void setStats(const CompileStats& stats);
    void animateIn();

    qreal revealProgress() const { return m_reveal; }
    void  setRevealProgress(qreal v);

protected:
    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void mousePressEvent(QMouseEvent*) override;

private:
    void drawBackground(QPainter& p);
    void drawHeader(QPainter& p);
    void drawGrade(QPainter& p);
    void drawStats(QPainter& p);
    void drawPhaseBar(QPainter& p);
    void drawOptimizationGain(QPainter& p);
    void drawFooter(QPainter& p);

    QString computeGrade() const;
    QColor  gradeColor() const;

    CompileStats m_stats;
    qreal        m_reveal = 0.0;
    QPropertyAnimation* m_anim = nullptr;
};
