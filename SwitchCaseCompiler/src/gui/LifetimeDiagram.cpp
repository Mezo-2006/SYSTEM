#include "LifetimeDiagram.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QFontMetrics>
#include <algorithm>
#include <set>

static const QList<QColor> PALETTE = {
    QColor("#6366F1"), QColor("#22C55E"), QColor("#F59E0B"),
    QColor("#EC4899"), QColor("#14B8A6"), QColor("#F97316"),
    QColor("#8B5CF6"), QColor("#06B6D4"), QColor("#EF4444"),
    QColor("#84CC16"),
};

QColor LifetimeDiagram::colorForVar(const std::string& name, int idx) {
    if (!name.empty() && name[0] == 't') return QColor("#94A3B8");
    return PALETTE[idx % PALETTE.size()];
}

LifetimeDiagram::LifetimeDiagram(QWidget* parent) : QWidget(parent) {
    setStyleSheet("background:#0F172A;");
    setMouseTracking(true);
    setMinimumHeight(300);
}

void LifetimeDiagram::reset() {
    vars.clear();
    instrs.clear();
    hoveredRow = -1;
    update();
}

void LifetimeDiagram::setInstructions(const std::vector<TACInstruction>& instructions) {
    instrs = instructions;
    buildLifetimes();
    int needed = std::max(300, (int)instrs.size() * 22 + 80);
    setMinimumHeight(needed);
    update();
}

void LifetimeDiagram::buildLifetimes() {
    vars.clear();
    std::map<std::string, int> varIdx;

    auto getOrCreate = [&](const std::string& name) -> VarLife& {
        if (varIdx.find(name) == varIdx.end()) {
            varIdx[name] = (int)vars.size();
            VarLife v;
            v.name   = name;
            v.isTemp = (!name.empty() && name[0] == 't');
            v.color  = colorForVar(name, (int)vars.size());
            vars.push_back(v);
        }
        return vars[varIdx[name]];
    };

    for (int i = 0; i < (int)instrs.size(); ++i) {
        const auto& inst = instrs[i];
        // Definition
        if (!inst.result.empty() && inst.result[0] != 'L') {
            auto& v = getOrCreate(inst.result);
            if (v.firstDef < 0) v.firstDef = i;
            v.lastUse = i;
        }
        // Uses
        for (const auto& arg : {inst.arg1, inst.arg2}) {
            if (!arg.empty() && !std::isdigit(arg[0]) && arg[0] != '"' && arg[0] != 'L') {
                auto& v = getOrCreate(arg);
                if (v.firstDef < 0) v.firstDef = i;
                v.lastUse = i;
            }
        }
    }

    // Sort: user vars first, then temps
    std::stable_sort(vars.begin(), vars.end(), [](const VarLife& a, const VarLife& b){
        if (a.isTemp != b.isTemp) return !a.isTemp;
        return a.name < b.name;
    });
}

void LifetimeDiagram::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.fillRect(rect(), QColor("#0F172A"));

    if (instrs.empty()) {
        p.setFont(QFont("Segoe UI", 11));
        p.setPen(QColor("#64748B"));
        p.drawText(rect(), Qt::AlignCenter, "Compile code to see variable lifetime diagram");
        return;
    }

    const int marginL  = 110;
    const int marginT  = 50;
    const int rowH     = 20;
    const int rowGap   = 2;
    const int instrW   = std::max(12, (width() - marginL - 20) / std::max(1, (int)instrs.size()));
    const int chartW   = instrW * (int)instrs.size();

    // ── Title ─────────────────────────────────────────────────────────────
    p.setFont(QFont("Segoe UI", 11, QFont::Bold));
    p.setPen(QColor("#7DD3FC"));
    p.drawText(marginL, 24, QString("Variable Lifetime Diagram  —  %1 variables  •  %2 instructions")
               .arg(vars.size()).arg(instrs.size()));

    // ── Instruction index header ──────────────────────────────────────────
    p.setFont(QFont("Consolas", 7));
    p.setPen(QColor("#475569"));
    for (int i = 0; i < (int)instrs.size(); ++i) {
        int x = marginL + i * instrW + instrW/2;
        if (i % 5 == 0) {
            p.drawText(QRectF(x-10, marginT-16, 20, 14), Qt::AlignCenter, QString::number(i+1));
            p.setPen(QPen(QColor("#1E293B"), 1));
            p.drawLine(x, marginT-4, x, marginT + (int)vars.size()*(rowH+rowGap));
            p.setPen(QColor("#475569"));
        }
    }

    // ── Rows ──────────────────────────────────────────────────────────────
    for (int vi = 0; vi < (int)vars.size(); ++vi) {
        const auto& v = vars[vi];
        int y = marginT + vi * (rowH + rowGap);
        bool hovered = (vi == hoveredRow);

        // Row background
        if (hovered) {
            p.fillRect(QRectF(0, y, width(), rowH), QColor(255,255,255,8));
        }

        // Variable name
        p.setFont(QFont("Consolas", 8, v.isTemp ? QFont::Normal : QFont::Bold));
        p.setPen(v.isTemp ? QColor("#64748B") : v.color.lighter(140));
        p.drawText(QRectF(4, y, marginL-8, rowH), Qt::AlignRight | Qt::AlignVCenter, 
                   QString::fromStdString(v.name));

        if (v.firstDef < 0 || v.lastUse < 0) continue;

        // Dead zone (before first def)
        if (v.firstDef > 0) {
            p.fillRect(QRectF(marginL, y+4, v.firstDef * instrW, rowH-8),
                       QColor(30,41,59));
        }

        // Live zone
        int liveX = marginL + v.firstDef * instrW;
        int liveW = (v.lastUse - v.firstDef + 1) * instrW;

        QLinearGradient lg(liveX, 0, liveX + liveW, 0);
        QColor c = v.color;
        c.setAlpha(hovered ? 200 : 140);
        lg.setColorAt(0, c.lighter(120));
        lg.setColorAt(1, c);
        p.setBrush(lg);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(QRectF(liveX, y+3, liveW, rowH-6), 4, 4);

        // Def marker (filled circle)
        p.setBrush(QColor("#F8FAFC"));
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(liveX + instrW*0.5, y + rowH*0.5), 4, 4);

        // Last use marker (diamond)
        int lastX = marginL + v.lastUse * instrW + instrW/2;
        QPolygonF diamond;
        diamond << QPointF(lastX, y+3)
                << QPointF(lastX+4, y+rowH/2)
                << QPointF(lastX, y+rowH-3)
                << QPointF(lastX-4, y+rowH/2);
        p.setBrush(QColor("#F8FAFC"));
        p.drawPolygon(diamond);

        // Dead zone (after last use)
        int afterX = marginL + (v.lastUse+1) * instrW;
        int afterW = chartW - (v.lastUse+1) * instrW;
        if (afterW > 0) {
            p.fillRect(QRectF(afterX, y+4, afterW, rowH-8), QColor(30,41,59));
        }

        // Hover tooltip
        if (hovered) {
            QString tip = QString("%1: alive %2→%3 (%4 instrs)")
                .arg(QString::fromStdString(v.name))
                .arg(v.firstDef+1).arg(v.lastUse+1)
                .arg(v.lastUse - v.firstDef + 1);
            p.setFont(QFont("Segoe UI", 8));
            p.setPen(QColor("#F8FAFC"));
            QRectF tipRect(liveX, y - 18, 260, 16);
            p.fillRect(tipRect.adjusted(-4,-2,4,2), QColor(30,41,59,220));
            p.drawText(tipRect, Qt::AlignLeft | Qt::AlignVCenter, tip);
        }
    }

    // ── Legend ────────────────────────────────────────────────────────────
    int legendY = marginT + (int)vars.size() * (rowH + rowGap) + 14;
    p.setFont(QFont("Segoe UI", 8));
    p.setPen(QColor("#64748B"));

    // Def marker
    p.setBrush(QColor("#F8FAFC")); p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(marginL + 8, legendY + 6), 4, 4);
    p.setPen(QColor("#94A3B8"));
    p.drawText(marginL + 16, legendY + 10, "First definition");

    // Last use marker
    QPolygonF ld;
    ld << QPointF(marginL+100, legendY+2) << QPointF(marginL+104, legendY+6)
       << QPointF(marginL+100, legendY+10) << QPointF(marginL+96, legendY+6);
    p.setBrush(QColor("#F8FAFC")); p.setPen(Qt::NoPen);
    p.drawPolygon(ld);
    p.setPen(QColor("#94A3B8"));
    p.drawText(marginL + 108, legendY + 10, "Last use");

    p.fillRect(QRectF(marginL+180, legendY+2, 20, 8), QColor(30,41,59));
    p.drawText(marginL + 204, legendY + 10, "Dead zone");

    setMinimumHeight(legendY + 30);
}

void LifetimeDiagram::mouseMoveEvent(QMouseEvent* event) {
    const int marginT = 50;
    const int rowH = 20, rowGap = 2;
    int y = event->pos().y() - marginT;
    int prev = hoveredRow;
    hoveredRow = (y >= 0) ? y / (rowH + rowGap) : -1;
    if (hoveredRow >= (int)vars.size()) hoveredRow = -1;
    if (hoveredRow != prev) update();
}

void LifetimeDiagram::leaveEvent(QEvent*) {
    hoveredRow = -1;
    update();
}
