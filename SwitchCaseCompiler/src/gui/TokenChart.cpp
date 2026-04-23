#include "TokenChart.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QFontMetrics>
#include <algorithm>
#include <map>

TokenChart::TokenChart(QWidget* parent) : QWidget(parent) {
    setStyleSheet("background:#0F172A;");
    setMouseTracking(true);
    setMinimumHeight(400);
}

QColor TokenChart::colorForType(const QString& type) {
    if (type == "SWITCH" || type == "CASE" || type == "DEFAULT" ||
        type == "BREAK"  || type == "RETURN")                    return QColor("#F472B6");
    if (type == "INT"    || type == "STRING" || type == "VOID")  return QColor("#34D399");
    if (type == "ID")                                            return QColor("#60A5FA");
    if (type == "CONSTANT")                                      return QColor("#FBBF24");
    if (type == "STRING_LITERAL")                                return QColor("#FB923C");
    if (type == "PLUS"   || type == "MINUS" || type == "MUL" ||
        type == "DIV"    || type == "ASSIGN")                    return QColor("#F87171");
    if (type == "LPAREN" || type == "RPAREN" || type == "LBRACE" ||
        type == "RBRACE" || type == "COLON"  || type == "SEMICOLON") return QColor("#94A3B8");
    if (type == "USING"  || type == "NAMESPACE" || type == "STD" ||
        type == "INCLUDE"|| type == "MAIN")                      return QColor("#A78BFA");
    if (type == "COUT"   || type == "CIN")                       return QColor("#6366F1");
    return QColor("#64748B");
}

void TokenChart::setTokens(const std::vector<Token>& tokens) {
    std::map<std::string, int> freq;
    totalTokens = 0;
    for (const auto& t : tokens) {
        if (t.type == TokenType::END_OF_FILE) continue;
        freq[t.typeToString()]++;
        totalTokens++;
    }

    bars.clear();
    maxCount = 1;
    for (const auto& [type, count] : freq) {
        bars.push_back({QString::fromStdString(type), count,
                        colorForType(QString::fromStdString(type)), {}});
        maxCount = std::max(maxCount, count);
    }
    // Sort descending
    std::sort(bars.begin(), bars.end(), [](const Bar& a, const Bar& b){
        return a.count > b.count;
    });

    setMinimumHeight(std::max(400, (int)bars.size() * 38 + 120));
    update();
}

void TokenChart::reset() {
    bars.clear();
    maxCount = 1;
    totalTokens = 0;
    hoveredIdx = -1;
    update();
}

void TokenChart::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.fillRect(rect(), QColor("#0F172A"));

    const int marginL = 130;
    const int marginR = 80;
    const int marginT = 60;
    const int barH    = 26;
    const int gap     = 12;
    const int chartW  = width() - marginL - marginR;

    // Title
    p.setFont(QFont("Segoe UI", 13, QFont::Bold));
    p.setPen(QColor("#38BDF8"));
    p.drawText(marginL, 28, QString("Token Frequency  —  %1 tokens total").arg(totalTokens));

    if (bars.empty()) {
        p.setFont(QFont("Segoe UI", 11));
        p.setPen(QColor("#64748B"));
        p.drawText(rect(), Qt::AlignCenter, "Compile code to see token frequency chart");
        return;
    }

    // Grid lines
    p.setPen(QPen(QColor("#1E293B"), 1));
    for (int i = 1; i <= 5; ++i) {
        int x = marginL + chartW * i / 5;
        p.drawLine(x, marginT - 10, x, marginT + (int)bars.size() * (barH + gap));
        p.setFont(QFont("Consolas", 8));
        p.setPen(QColor("#475569"));
        p.drawText(x - 15, marginT - 14, QString::number(maxCount * i / 5));
        p.setPen(QPen(QColor("#1E293B"), 1));
    }

    // Bars
    for (int i = 0; i < (int)bars.size(); ++i) {
        auto& bar = bars[i];
        int y = marginT + i * (barH + gap);
        qreal barW = (qreal)bar.count / maxCount * chartW;

        bar.rect = QRectF(marginL, y, barW, barH);

        bool hovered = (i == hoveredIdx);

        // Label
        p.setFont(QFont("Consolas", 9, hovered ? QFont::Bold : QFont::Normal));
        p.setPen(hovered ? bar.color.lighter(150) : QColor("#94A3B8"));
        QRectF labelRect(0, y, marginL - 8, barH);
        p.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, bar.label);

        // Bar shadow
        p.setBrush(QColor(0,0,0,40));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(bar.rect.translated(2,2), 4, 4);

        // Bar gradient
        QLinearGradient grad(bar.rect.left(), 0, bar.rect.right(), 0);
        QColor c = bar.color;
        if (hovered) c = c.lighter(130);
        grad.setColorAt(0, c.lighter(120));
        grad.setColorAt(1, c.darker(110));
        p.setBrush(grad);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(bar.rect, 4, 4);

        // Count label
        p.setFont(QFont("Consolas", 9, QFont::Bold));
        p.setPen(Qt::white);
        if (barW > 30) {
            p.drawText(QRectF(marginL + barW - 36, y, 34, barH),
                       Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(bar.count));
        } else {
            p.setPen(bar.color.lighter(160));
            p.drawText(QRectF(marginL + barW + 4, y, 40, barH),
                       Qt::AlignLeft | Qt::AlignVCenter,
                       QString::number(bar.count));
        }

        // Percentage on hover
        if (hovered && totalTokens > 0) {
            qreal pct = (qreal)bar.count / totalTokens * 100.0;
            p.setFont(QFont("Segoe UI", 9));
            p.setPen(bar.color.lighter(180));
            p.drawText(QRectF(marginL + barW + 44, y, 80, barH),
                       Qt::AlignLeft | Qt::AlignVCenter,
                       QString("%1%").arg(pct, 0, 'f', 1));
        }
    }
}

void TokenChart::mouseMoveEvent(QMouseEvent* event) {
    int prev = hoveredIdx;
    hoveredIdx = -1;
    for (int i = 0; i < (int)bars.size(); ++i) {
        if (bars[i].rect.adjusted(-4, -4, 60, 4).contains(event->pos())) {
            hoveredIdx = i;
            break;
        }
    }
    if (hoveredIdx != prev) update();
}

void TokenChart::leaveEvent(QEvent*) {
    hoveredIdx = -1;
    update();
}
