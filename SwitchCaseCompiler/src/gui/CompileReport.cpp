#include "CompileReport.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFontMetrics>
#include <QScreen>
#include <QApplication>
#include <cmath>
#include <algorithm>

CompileReport::CompileReport(QWidget* parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::Dialog)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(true);

    // Size: 780 x 560
    resize(780, 560);

    // Centre on parent
    if (parent) {
        QPoint centre = parent->geometry().center();
        move(centre.x() - 390, centre.y() - 280);
    }

    m_anim = new QPropertyAnimation(this, "revealProgress", this);
    m_anim->setDuration(900);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);
}

void CompileReport::setStats(const CompileStats& stats) {
    m_stats = stats;
}

void CompileReport::animateIn() {
    m_reveal = 0.0;
    m_anim->setStartValue(0.0);
    m_anim->setEndValue(1.0);
    m_anim->start();
    show();
    raise();
    activateWindow();
}

void CompileReport::setRevealProgress(qreal v) {
    m_reveal = v;
    update();
}

// ─────────────────────────────────────────────────────────────────────────────
QString CompileReport::computeGrade() const {
    if (!m_stats.success) return "F";
    int score = 100;
    // Deduct for high TAC count
    if (m_stats.tacBefore > 40) score -= 10;
    // Reward optimization
    if (m_stats.tacBefore > 0) {
        double gain = 1.0 - (double)m_stats.tacAfter / m_stats.tacBefore;
        if (gain > 0.3) score += 5;
    }
    // Reward derivation steps (more = more complex = more impressive)
    if (m_stats.derivationSteps > 30) score += 5;
    score = std::min(100, score);
    if (score >= 95) return "A+";
    if (score >= 90) return "A";
    if (score >= 80) return "B+";
    if (score >= 70) return "B";
    return "C";
}

QColor CompileReport::gradeColor() const {
    QString g = computeGrade();
    if (g == "A+" || g == "A") return QColor("#22C55E");
    if (g == "B+" || g == "B") return QColor("#3B82F6");
    if (g == "F")               return QColor("#EF4444");
    return QColor("#F59E0B");
}

// ─────────────────────────────────────────────────────────────────────────────
void CompileReport::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    // Clip to rounded rect
    QPainterPath clip;
    clip.addRoundedRect(rect(), 20, 20);
    p.setClipPath(clip);

    drawBackground(p);
    if (m_reveal < 0.05) return;
    drawHeader(p);
    if (m_reveal < 0.2) return;
    drawGrade(p);
    if (m_reveal < 0.35) return;
    drawStats(p);
    if (m_reveal < 0.6) return;
    drawPhaseBar(p);
    if (m_reveal < 0.8) return;
    drawOptimizationGain(p);
    drawFooter(p);
}

void CompileReport::drawBackground(QPainter& p) {
    // Deep dark gradient
    QLinearGradient bg(0, 0, width(), height());
    bg.setColorAt(0.0, QColor("#0F172A"));
    bg.setColorAt(0.5, QColor("#1E1B4B"));
    bg.setColorAt(1.0, QColor("#0F172A"));
    p.fillRect(rect(), bg);

    // Subtle radial glow top-right
    QRadialGradient glow(width() * 0.8, 0, width() * 0.6);
    glow.setColorAt(0, QColor(99, 102, 241, 40));
    glow.setColorAt(1, Qt::transparent);
    p.fillRect(rect(), glow);

    // Border
    p.setPen(QPen(QColor(99, 102, 241, 120), 1.5));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(rect().adjusted(1,1,-1,-1), 20, 20);

    // Dot grid
    p.setPen(QPen(QColor(255,255,255,8), 1));
    for (int x = 0; x < width(); x += 32)
        for (int y = 0; y < height(); y += 32)
            p.drawPoint(x, y);
}

void CompileReport::drawHeader(QPainter& p) {
    qreal alpha = std::min(1.0, (m_reveal - 0.05) / 0.15);

    // Icon + title
    p.setFont(QFont("Segoe UI Emoji", 22));
    p.setPen(QColor(255,255,255, static_cast<int>(255*alpha)));
    p.drawText(QRectF(30, 22, 50, 50), Qt::AlignCenter,
               m_stats.success ? "✅" : "❌");

    p.setFont(QFont("Segoe UI", 18, QFont::Bold));
    p.setPen(QColor(248, 250, 252, static_cast<int>(255*alpha)));
    p.drawText(QRectF(82, 22, 500, 30), Qt::AlignLeft | Qt::AlignVCenter,
               m_stats.success ? "Compilation Successful" : "Compilation Failed");

    p.setFont(QFont("Segoe UI", 10));
    p.setPen(QColor(148, 163, 184, static_cast<int>(200*alpha)));
    p.drawText(QRectF(82, 52, 500, 20), Qt::AlignLeft | Qt::AlignVCenter,
               QString("Total time: %1 ms  •  %2 source lines  •  %3 tokens")
               .arg(m_stats.totalMs).arg(m_stats.sourceLines).arg(m_stats.tokenCount));

    // Divider
    p.setPen(QPen(QColor(99,102,241, static_cast<int>(80*alpha)), 1));
    p.drawLine(30, 78, width()-30, 78);

    // Close hint
    p.setFont(QFont("Segoe UI", 9));
    p.setPen(QColor(100,116,139, static_cast<int>(180*alpha)));
    p.drawText(QRectF(width()-160, 22, 140, 20), Qt::AlignRight | Qt::AlignVCenter,
               "Press any key to close");
}

void CompileReport::drawGrade(QPainter& p) {
    qreal alpha = std::min(1.0, (m_reveal - 0.2) / 0.15);
    QString grade = computeGrade();
    QColor gc = gradeColor();

    // Grade circle
    QRectF circle(width()-130, 90, 100, 100);
    QRadialGradient rg(circle.center(), 50);
    QColor gc2 = gc; gc2.setAlpha(static_cast<int>(60*alpha));
    rg.setColorAt(0, gc2);
    rg.setColorAt(1, Qt::transparent);
    p.setBrush(rg);
    p.setPen(Qt::NoPen);
    p.drawEllipse(circle.adjusted(-10,-10,10,10));

    p.setBrush(Qt::NoBrush);
    QColor border = gc; border.setAlpha(static_cast<int>(200*alpha));
    p.setPen(QPen(border, 2));
    p.drawEllipse(circle);

    p.setFont(QFont("Segoe UI", 32, QFont::Bold));
    QColor tc = gc; tc.setAlpha(static_cast<int>(255*alpha));
    p.setPen(tc);
    p.drawText(circle, Qt::AlignCenter, grade);

    p.setFont(QFont("Segoe UI", 9));
    p.setPen(QColor(148,163,184, static_cast<int>(180*alpha)));
    p.drawText(QRectF(width()-130, 195, 100, 16), Qt::AlignCenter, "Compile Score");
}

void CompileReport::drawStats(QPainter& p) {
    qreal alpha = std::min(1.0, (m_reveal - 0.35) / 0.25);

    struct StatItem { QString icon; QString label; QString value; QColor color; };
    QList<StatItem> items = {
        {"🔤", "Tokens",          QString::number(m_stats.tokenCount),      QColor("#38BDF8")},
        {"🌲", "Derivation Steps",QString::number(m_stats.derivationSteps), QColor("#A78BFA")},
        {"📊", "Symbols",         QString::number(m_stats.symbolCount),     QColor("#34D399")},
        {"🔄", "TAC Instructions",QString::number(m_stats.tacAfter),        QColor("#FBBF24")},
        {"⚙️",  "Assembly Lines",  QString::number(m_stats.assemblyLines),   QColor("#F472B6")},
        {"🎯", "Cases",           QString::number(m_stats.caseCount),       QColor("#FB923C")},
    };

    const int cols = 3;
    const int cardW = 200, cardH = 64;
    const int startX = 30, startY = 92;
    const int gapX = 14, gapY = 10;

    for (int i = 0; i < items.size(); ++i) {
        int col = i % cols, row = i / cols;
        int x = startX + col * (cardW + gapX);
        int y = startY + row * (cardH + gapY);

        // Stagger animation
        qreal itemAlpha = std::max(0.0, std::min(1.0, (alpha - i * 0.08) / 0.3));
        if (itemAlpha <= 0) continue;

        QRectF card(x, y, cardW, cardH);

        // Card background
        QLinearGradient cg(card.topLeft(), card.bottomRight());
        QColor c1 = items[i].color; c1.setAlpha(static_cast<int>(25*itemAlpha));
        QColor c2 = items[i].color; c2.setAlpha(static_cast<int>(10*itemAlpha));
        cg.setColorAt(0, c1); cg.setColorAt(1, c2);
        p.setBrush(cg);
        QColor border = items[i].color; border.setAlpha(static_cast<int>(60*itemAlpha));
        p.setPen(QPen(border, 1));
        p.drawRoundedRect(card, 10, 10);

        // Icon
        p.setFont(QFont("Segoe UI Emoji", 14));
        p.setPen(QColor(255,255,255, static_cast<int>(220*itemAlpha)));
        p.drawText(QRectF(x+10, y+8, 30, 48), Qt::AlignCenter, items[i].icon);

        // Value
        p.setFont(QFont("Segoe UI", 20, QFont::Bold));
        QColor vc = items[i].color; vc.setAlpha(static_cast<int>(255*itemAlpha));
        p.setPen(vc);
        p.drawText(QRectF(x+46, y+8, cardW-56, 28), Qt::AlignLeft | Qt::AlignVCenter,
                   items[i].value);

        // Label
        p.setFont(QFont("Segoe UI", 8));
        p.setPen(QColor(148,163,184, static_cast<int>(180*itemAlpha)));
        p.drawText(QRectF(x+46, y+36, cardW-56, 18), Qt::AlignLeft | Qt::AlignVCenter,
                   items[i].label);
    }
}

void CompileReport::drawPhaseBar(QPainter& p) {
    qreal alpha = std::min(1.0, (m_reveal - 0.6) / 0.2);
    if (alpha <= 0) return;

    int y = 250;
    p.setFont(QFont("Segoe UI", 9, QFont::Bold));
    p.setPen(QColor(148,163,184, static_cast<int>(200*alpha)));
    p.drawText(QRectF(30, y, 300, 16), Qt::AlignLeft | Qt::AlignVCenter,
               "Phase Timing Breakdown");
    y += 20;

    const QList<QColor> phaseColors = {
        QColor("#38BDF8"), QColor("#A78BFA"), QColor("#34D399"),
        QColor("#FBBF24"), QColor("#F97316"), QColor("#F472B6"), QColor("#6366F1")
    };
    const QStringList phaseShort = {"Lex","Parse","Sem","TAC","Opt","CGen","TgtOpt"};

    qint64 maxMs = 1;
    for (auto ms : m_stats.phaseMs) maxMs = std::max(maxMs, ms);

    const int barAreaW = width() - 60;
    const int barH = 18;
    const int gap = 6;

    for (int i = 0; i < (int)m_stats.phaseMs.size() && i < 7; ++i) {
        qreal itemAlpha = std::max(0.0, std::min(1.0, (alpha - i*0.06)/0.2));
        if (itemAlpha <= 0) continue;

        qreal barW = (qreal)m_stats.phaseMs[i] / maxMs * (barAreaW - 120) * itemAlpha;
        barW = std::max(barW, 4.0);

        // Label
        p.setFont(QFont("Consolas", 8));
        p.setPen(QColor(148,163,184, static_cast<int>(200*itemAlpha)));
        p.drawText(QRectF(30, y, 60, barH), Qt::AlignRight | Qt::AlignVCenter,
                   phaseShort[i]);

        // Bar
        QRectF bar(96, y+2, barW, barH-4);
        QLinearGradient bg(bar.left(), 0, bar.right(), 0);
        QColor c = phaseColors[i]; c.setAlpha(static_cast<int>(200*itemAlpha));
        bg.setColorAt(0, c.lighter(120));
        bg.setColorAt(1, c);
        p.setBrush(bg);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(bar, 3, 3);

        // Ms label
        p.setFont(QFont("Consolas", 8));
        p.setPen(QColor(248,250,252, static_cast<int>(200*itemAlpha)));
        p.drawText(QRectF(96 + barW + 6, y, 60, barH),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   QString("%1ms").arg(m_stats.phaseMs[i]));

        y += barH + gap;
    }
}

void CompileReport::drawOptimizationGain(QPainter& p) {
    qreal alpha = std::min(1.0, (m_reveal - 0.8) / 0.2);
    if (alpha <= 0 || m_stats.tacBefore == 0) return;

    double gain = 1.0 - (double)m_stats.tacAfter / m_stats.tacBefore;
    int pct = static_cast<int>(gain * 100);

    int x = width() - 200, y = 250;
    p.setFont(QFont("Segoe UI", 9, QFont::Bold));
    p.setPen(QColor(148,163,184, static_cast<int>(200*alpha)));
    p.drawText(QRectF(x, y, 170, 16), Qt::AlignLeft, "Optimization Gain");
    y += 22;

    // Donut chart
    QRectF donut(x+35, y, 100, 100);
    int startAngle = 90 * 16;
    int spanAngle  = -static_cast<int>(gain * 360 * 16 * alpha);

    // Background arc
    p.setPen(QPen(QColor(30,41,59), 14, Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    p.drawArc(donut, 0, 360*16);

    // Gain arc
    QColor arcColor = gain > 0.2 ? QColor("#22C55E") : QColor("#F59E0B");
    arcColor.setAlpha(static_cast<int>(230*alpha));
    p.setPen(QPen(arcColor, 14, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(donut, startAngle, spanAngle);

    // Centre text
    p.setFont(QFont("Segoe UI", 16, QFont::Bold));
    p.setPen(QColor(248,250,252, static_cast<int>(255*alpha)));
    p.drawText(donut, Qt::AlignCenter, QString("%1%").arg(pct));

    p.setFont(QFont("Segoe UI", 8));
    p.setPen(QColor(148,163,184, static_cast<int>(180*alpha)));
    p.drawText(QRectF(x, y+106, 170, 16), Qt::AlignCenter,
               QString("%1 → %2 instructions").arg(m_stats.tacBefore).arg(m_stats.tacAfter));
}

void CompileReport::drawFooter(QPainter& p) {
    qreal alpha = std::min(1.0, (m_reveal - 0.85) / 0.15);
    if (alpha <= 0) return;

    // Divider
    p.setPen(QPen(QColor(99,102,241, static_cast<int>(60*alpha)), 1));
    p.drawLine(30, height()-40, width()-30, height()-40);

    p.setFont(QFont("Segoe UI", 9));
    p.setPen(QColor(100,116,139, static_cast<int>(180*alpha)));
    p.drawText(QRectF(30, height()-32, width()-60, 20),
               Qt::AlignCenter,
               "Switch-Case Compiler  •  Educational Tool  •  All 7 phases completed");
}

void CompileReport::keyPressEvent(QKeyEvent*) { close(); }
void CompileReport::mousePressEvent(QMouseEvent*) { close(); }
