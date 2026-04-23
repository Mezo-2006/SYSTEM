#include "PipelineDiagram.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QResizeEvent>
#include <QFontMetrics>
#include <cmath>

PipelineDiagram::PipelineDiagram(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(260);
    setStyleSheet("background:#0F172A;");

    phases = {
        {"Lexer",     "🔤", "#0EA5E9", -1, "pending", 0, "tokens"},
        {"Parser",    "🌲", "#8B5CF6", -1, "pending", 0, "steps"},
        {"Semantic",  "📊", "#F59E0B", -1, "pending", 0, "symbols"},
        {"TAC Gen",   "🔄", "#10B981", -1, "pending", 0, "instrs"},
        {"Optimizer", "⚡", "#F97316", -1, "pending", 0, "instrs"},
        {"CodeGen",   "⚙️",  "#EC4899", -1, "pending", 0, "lines"},
        {"Target Opt","🎯", "#6366F1", -1, "pending", 0, "lines"},
    };
    arrowProgress.assign(phases.size() - 1, 0.0);
    boxes.resize(phases.size());

    m_anim = new QPropertyAnimation(this, "particleProgress", this);
    m_anim->setDuration(600);
    m_anim->setEasingCurve(QEasingCurve::InOutCubic);

    m_ticker = new QTimer(this);
    m_ticker->setInterval(16);
    connect(m_ticker, &QTimer::timeout, this, [this]{ update(); });
    m_ticker->start();
}

void PipelineDiagram::resetAll() {
    for (auto& p : phases) { p.status = "pending"; p.durationMs = -1; p.outputCount = 0; }
    arrowProgress.assign(phases.size() - 1, 0.0);
    m_particleFrom = -1;
    update();
}

void PipelineDiagram::setPhaseStatus(int idx, const QString& status, qint64 ms, int outputCount) {
    if (idx < 0 || idx >= (int)phases.size()) return;
    phases[idx].status      = status;
    phases[idx].durationMs  = ms;
    phases[idx].outputCount = outputCount;

    if (status == "done" && idx < (int)arrowProgress.size()) {
        // Animate arrow to next phase
        arrowProgress[idx] = 0.0;
        startParticle(idx);
    }
    update();
}

void PipelineDiagram::startParticle(int fromPhase) {
    m_particleFrom = fromPhase;
    m_anim->stop();
    m_anim->setStartValue(0.0);
    m_anim->setEndValue(1.0);
    m_anim->start();
    connect(m_anim, &QPropertyAnimation::finished, this, [this]{
        if (m_particleFrom >= 0 && m_particleFrom < (int)arrowProgress.size())
            arrowProgress[m_particleFrom] = 1.0;
        m_particleFrom = -1;
        update();
    }, Qt::UniqueConnection);
}

void PipelineDiagram::setParticleProgress(qreal v) {
    m_particle = v;
    if (m_particleFrom >= 0 && m_particleFrom < (int)arrowProgress.size())
        arrowProgress[m_particleFrom] = v;
    update();
}

// ─────────────────────────────────────────────────────────────────────────────
void PipelineDiagram::resizeEvent(QResizeEvent*) {
    const int n = (int)phases.size();
    const qreal margin = 30;
    const qreal totalW = width() - 2 * margin;
    const qreal nodeW  = std::min(110.0, totalW / n - 12);
    const qreal nodeH  = 90.0;
    const qreal gap    = (totalW - n * nodeW) / (n - 1);
    const qreal y      = (height() - nodeH) * 0.5;

    for (int i = 0; i < n; ++i)
        boxes[i] = QRectF(margin + i * (nodeW + gap), y, nodeW, nodeH);
}

void PipelineDiagram::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    // Background
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0, QColor("#0F172A"));
    bg.setColorAt(1, QColor("#1E293B"));
    p.fillRect(rect(), bg);

    // Subtle grid
    p.setPen(QPen(QColor(255,255,255,8), 1));
    for (int x = 0; x < width(); x += 40) p.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += 40) p.drawLine(0, y, width(), y);

    // Title
    p.setPen(QColor(148, 163, 184));
    p.setFont(QFont("Segoe UI", 9));
    p.drawText(12, 18, "Compilation Pipeline  •  Animated data flow");

    if (boxes.empty() || boxes[0].isNull()) return;

    // Arrows
    for (int i = 0; i < (int)phases.size() - 1; ++i)
        drawArrow(p, boxes[i], boxes[i+1], arrowProgress[i], QColor(phases[i].color));

    // Nodes
    for (int i = 0; i < (int)phases.size(); ++i)
        drawPhaseNode(p, i, boxes[i]);

    // Particle
    drawParticle(p);
}

void PipelineDiagram::drawPhaseNode(QPainter& p, int idx, const QRectF& box) {
    const PhaseInfo& ph = phases[idx];
    QColor base(ph.color);

    // Glow for active
    if (ph.status == "active") {
        QRadialGradient glow(box.center(), box.width() * 0.7);
        QColor gc = base; gc.setAlpha(60);
        glow.setColorAt(0, gc);
        glow.setColorAt(1, Qt::transparent);
        p.setBrush(glow);
        p.setPen(Qt::NoPen);
        p.drawEllipse(box.adjusted(-12,-12,12,12));
    }

    // Shadow
    p.setBrush(QColor(0,0,0,50));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(box.translated(3,4), 12, 12);

    // Body gradient
    QLinearGradient grad(box.topLeft(), box.bottomRight());
    if (ph.status == "done") {
        grad.setColorAt(0, base.lighter(130));
        grad.setColorAt(1, base.darker(110));
    } else if (ph.status == "failed") {
        grad.setColorAt(0, QColor("#7F1D1D"));
        grad.setColorAt(1, QColor("#450A0A"));
    } else if (ph.status == "active") {
        grad.setColorAt(0, base.lighter(150));
        grad.setColorAt(1, base);
    } else {
        grad.setColorAt(0, QColor("#1E293B"));
        grad.setColorAt(1, QColor("#0F172A"));
    }
    p.setBrush(grad);

    QColor border = (ph.status == "pending") ? QColor("#334155") : base;
    p.setPen(QPen(border, ph.status == "active" ? 2.5 : 1.5));
    p.drawRoundedRect(box, 12, 12);

    // Icon
    p.setFont(QFont("Segoe UI Emoji", 16));
    p.setPen(Qt::white);
    QRectF iconRect(box.left(), box.top() + 6, box.width(), 26);
    p.drawText(iconRect, Qt::AlignCenter, ph.icon);

    // Name
    p.setFont(QFont("Segoe UI", 8, QFont::Bold));
    p.setPen(ph.status == "pending" ? QColor("#64748B") : Qt::white);
    QRectF nameRect(box.left(), box.top() + 34, box.width(), 16);
    p.drawText(nameRect, Qt::AlignCenter, ph.name);

    // Duration / status
    QString sub;
    if (ph.status == "done" && ph.durationMs >= 0)
        sub = QString("%1ms").arg(ph.durationMs);
    else if (ph.status == "active")
        sub = "running…";
    else if (ph.status == "failed")
        sub = "FAILED";

    if (!sub.isEmpty()) {
        p.setFont(QFont("Consolas", 7));
        p.setPen(ph.status == "failed" ? QColor("#FCA5A5") : QColor(base).lighter(160));
        QRectF subRect(box.left(), box.top() + 52, box.width(), 14);
        p.drawText(subRect, Qt::AlignCenter, sub);
    }

    // Output count badge
    if (ph.status == "done" && ph.outputCount > 0) {
        QString badge = QString::number(ph.outputCount) + " " + ph.outputUnit;
        p.setFont(QFont("Segoe UI", 7));
        p.setPen(QColor(base).lighter(180));
        QRectF badgeRect(box.left(), box.top() + 66, box.width(), 14);
        p.drawText(badgeRect, Qt::AlignCenter, badge);
    }
}

void PipelineDiagram::drawArrow(QPainter& p, const QRectF& from, const QRectF& to,
                                 qreal progress, const QColor& col) {
    if (progress <= 0.0) {
        // Draw faint dashed line
        p.setPen(QPen(QColor(51,65,85), 1.5, Qt::DashLine));
        p.drawLine(QPointF(from.right(), from.center().y()),
                   QPointF(to.left(),   to.center().y()));
        return;
    }

    qreal x1 = from.right(), y1 = from.center().y();
    qreal x2 = to.left(),    y2 = to.center().y();
    qreal xEnd = x1 + (x2 - x1) * progress;

    // Glow
    QPen glowPen(col, 6);
    glowPen.setCapStyle(Qt::RoundCap);
    QColor gc = col; gc.setAlpha(40);
    glowPen.setColor(gc);
    p.setPen(glowPen);
    p.drawLine(QPointF(x1, y1), QPointF(xEnd, y2));

    // Main line
    QLinearGradient lg(x1, y1, xEnd, y2);
    lg.setColorAt(0, col.lighter(130));
    lg.setColorAt(1, col);
    p.setPen(QPen(QBrush(lg), 2.5, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(x1, y1), QPointF(xEnd, y2));

    // Arrow head at end
    if (progress >= 0.95) {
        double angle = std::atan2(y2 - y1, x2 - x1);
        double as = 8;
        QPointF a1(xEnd - as * std::cos(angle - M_PI/6),
                   y2   - as * std::sin(angle - M_PI/6));
        QPointF a2(xEnd - as * std::cos(angle + M_PI/6),
                   y2   - as * std::sin(angle + M_PI/6));
        QPolygonF head;
        head << QPointF(xEnd, y2) << a1 << a2;
        p.setBrush(col);
        p.setPen(Qt::NoPen);
        p.drawPolygon(head);
    }
}

void PipelineDiagram::drawParticle(QPainter& p) {
    if (m_particleFrom < 0 || m_particleFrom >= (int)boxes.size() - 1) return;
    const QRectF& from = boxes[m_particleFrom];
    const QRectF& to   = boxes[m_particleFrom + 1];

    qreal x = from.right() + (to.left() - from.right()) * m_particle;
    qreal y = from.center().y();

    QColor col(phases[m_particleFrom].color);
    QRadialGradient rg(x, y, 10);
    rg.setColorAt(0, col.lighter(180));
    rg.setColorAt(0.5, col);
    rg.setColorAt(1, Qt::transparent);
    p.setBrush(rg);
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(x, y), 10, 10);
}
