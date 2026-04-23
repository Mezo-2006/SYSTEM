#include "ShortcutOverlay.h"
#include <QPainter>
#include <QPainterPath>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QLinearGradient>
#include <QFontMetrics>

struct ShortcutEntry {
    QString keys;
    QString description;
    QString category;
};

static const QList<ShortcutEntry> SHORTCUTS = {
    // Editor
    {"Ctrl+Z",       "Undo",                          "Editor"},
    {"Ctrl+Y",       "Redo",                          "Editor"},
    {"Ctrl+Space",   "Auto-complete",                 "Editor"},
    {"Tab",          "Insert 4 spaces",               "Editor"},
    {"Ctrl+A",       "Select all",                    "Editor"},
    // Compile
    {"F5",           "Compile Pipeline",              "Compile"},
    {"Ctrl+Enter",   "Compile Pipeline",              "Compile"},
    // Navigation
    {"Ctrl+Tab",     "Next tab",                      "Navigate"},
    {"Ctrl+Shift+Tab","Previous tab",                 "Navigate"},
    {"Ctrl+1..9",    "Jump to tab N",                 "Navigate"},
    // Views
    {"Ctrl+/",       "Toggle this overlay",           "Help"},
    {"Ctrl+?",       "Toggle this overlay",           "Help"},
    {"Escape",       "Close overlay / dialog",        "Help"},
    // Parse Tree
    {"Scroll",       "Zoom parse/AST tree",           "Tree"},
    {"Right-drag",   "Pan parse/AST tree",            "Tree"},
    {"Click node",   "Jump to derivation step",       "Tree"},
    // Simulator
    {"Step ▶",       "Execute one TAC instruction",   "Simulator"},
    {"◀ Back",       "Undo last step",                "Simulator"},
    {"▶ Auto",       "Auto-play execution",           "Simulator"},
};

static const QMap<QString, QColor> CAT_COLORS = {
    {"Editor",    QColor("#38BDF8")},
    {"Compile",   QColor("#22C55E")},
    {"Navigate",  QColor("#A78BFA")},
    {"Help",      QColor("#F59E0B")},
    {"Tree",      QColor("#34D399")},
    {"Simulator", QColor("#F472B6")},
};

ShortcutOverlay::ShortcutOverlay(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::StrongFocus);

    m_anim = new QPropertyAnimation(this, "opacity", this);
    m_anim->setDuration(220);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);

    hide();
}

void ShortcutOverlay::setOpacity(qreal v) {
    m_opacity = v;
    update();
    if (v <= 0.0 && !m_showing) hide();
}

void ShortcutOverlay::toggleVisible() {
    m_showing = !m_showing;
    if (m_showing) {
        raise();
        show();
        setFocus();
        m_anim->stop();
        m_anim->setStartValue(m_opacity);
        m_anim->setEndValue(1.0);
        m_anim->start();
    } else {
        m_anim->stop();
        m_anim->setStartValue(m_opacity);
        m_anim->setEndValue(0.0);
        m_anim->start();
        connect(m_anim, &QPropertyAnimation::finished, this, [this]{
            if (!m_showing) hide();
        }, Qt::UniqueConnection);
    }
}

void ShortcutOverlay::paintEvent(QPaintEvent*) {
    if (m_opacity <= 0.01) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    // Full-screen dim
    p.fillRect(rect(), QColor(0, 0, 0, static_cast<int>(180 * m_opacity)));

    // Card dimensions
    const int cardW = 720, cardH = 520;
    const int cardX = (width()  - cardW) / 2;
    const int cardY = (height() - cardH) / 2;
    QRectF card(cardX, cardY, cardW, cardH);

    // Card shadow
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, static_cast<int>(80 * m_opacity)));
    p.drawRoundedRect(card.translated(4, 6), 18, 18);

    // Card background
    QLinearGradient bg(card.topLeft(), card.bottomRight());
    bg.setColorAt(0.0, QColor(15, 23, 42, static_cast<int>(252 * m_opacity)));
    bg.setColorAt(1.0, QColor(30, 27, 75, static_cast<int>(252 * m_opacity)));
    p.setBrush(bg);
    p.setPen(QPen(QColor(99, 102, 241, static_cast<int>(120 * m_opacity)), 1.5));
    p.drawRoundedRect(card, 18, 18);

    // Subtle dot grid inside card
    p.setPen(QPen(QColor(255,255,255, static_cast<int>(6 * m_opacity)), 1));
    for (int x = cardX; x < cardX+cardW; x += 28)
        for (int y = cardY; y < cardY+cardH; y += 28)
            p.drawPoint(x, y);

    // ── Header ────────────────────────────────────────────────────────────
    p.setFont(QFont("Segoe UI Emoji", 18));
    p.setPen(QColor(255,255,255, static_cast<int>(230 * m_opacity)));
    p.drawText(QRectF(cardX+20, cardY+18, 50, 40), Qt::AlignCenter, "⌨️");

    p.setFont(QFont("Segoe UI", 16, QFont::Bold));
    p.setPen(QColor(248,250,252, static_cast<int>(240 * m_opacity)));
    p.drawText(QRectF(cardX+70, cardY+18, 400, 40),
               Qt::AlignLeft | Qt::AlignVCenter, "Keyboard Shortcuts");

    p.setFont(QFont("Segoe UI", 9));
    p.setPen(QColor(100,116,139, static_cast<int>(200 * m_opacity)));
    p.drawText(QRectF(cardX+cardW-200, cardY+18, 180, 40),
               Qt::AlignRight | Qt::AlignVCenter, "Press Esc or click to close");

    // Divider
    p.setPen(QPen(QColor(99,102,241, static_cast<int>(60 * m_opacity)), 1));
    p.drawLine(cardX+20, cardY+62, cardX+cardW-20, cardY+62);

    // ── Shortcut grid (3 columns) ─────────────────────────────────────────
    const int cols    = 3;
    const int colW    = (cardW - 40) / cols;
    const int rowH    = 28;
    const int startY  = cardY + 74;
    const int startX  = cardX + 14;

    // Group by category
    QMap<QString, QList<ShortcutEntry>> grouped;
    for (const auto& s : SHORTCUTS) grouped[s.category].append(s);

    // Flatten into column-major layout
    QList<ShortcutEntry> flat;
    for (const auto& cat : {"Editor","Compile","Navigate","Help","Tree","Simulator"}) {
        if (grouped.contains(cat)) {
            // Insert a category header as a special entry
            flat.append({cat, "", "__header__"});
            for (const auto& e : grouped[cat]) flat.append(e);
        }
    }

    int col = 0, row = 0;
    const int maxRows = 16;

    for (const auto& entry : flat) {
        int x = startX + col * colW;
        int y = startY + row * rowH;

        if (y + rowH > cardY + cardH - 40) {
            col++;
            row = 0;
            if (col >= cols) break;
            x = startX + col * colW;
            y = startY;
        }

        if (entry.description == "__header__") {
            // Category label
            QColor catCol = CAT_COLORS.value(entry.keys, QColor("#94A3B8"));
            catCol.setAlpha(static_cast<int>(200 * m_opacity));
            p.setFont(QFont("Segoe UI", 8, QFont::Bold));
            p.setPen(catCol);
            p.drawText(QRectF(x+4, y+2, colW-8, rowH-4),
                       Qt::AlignLeft | Qt::AlignVCenter,
                       entry.keys.toUpper());

            // Category underline
            p.setPen(QPen(catCol, 1));
            p.drawLine(x+4, y+rowH-2, x+colW-8, y+rowH-2);
            row++;
            continue;
        }

        // Key badge
        QColor catCol = CAT_COLORS.value(entry.category, QColor("#6366F1"));
        QRectF badge(x+4, y+5, 90, rowH-10);
        QColor badgeBg = catCol; badgeBg.setAlpha(static_cast<int>(30 * m_opacity));
        QColor badgeBorder = catCol; badgeBorder.setAlpha(static_cast<int>(80 * m_opacity));
        p.setBrush(badgeBg);
        p.setPen(QPen(badgeBorder, 1));
        p.drawRoundedRect(badge, 5, 5);

        p.setFont(QFont("Consolas", 8, QFont::Bold));
        QColor keyCol = catCol.lighter(160); keyCol.setAlpha(static_cast<int>(230 * m_opacity));
        p.setPen(keyCol);
        p.drawText(badge, Qt::AlignCenter, entry.keys);

        // Description
        p.setFont(QFont("Segoe UI", 9));
        p.setPen(QColor(203,213,225, static_cast<int>(200 * m_opacity)));
        p.drawText(QRectF(x+100, y+2, colW-108, rowH-4),
                   Qt::AlignLeft | Qt::AlignVCenter, entry.description);

        row++;
        if (row >= maxRows) { col++; row = 0; }
    }

    // ── Footer ────────────────────────────────────────────────────────────
    p.setPen(QPen(QColor(99,102,241, static_cast<int>(40 * m_opacity)), 1));
    p.drawLine(cardX+20, cardY+cardH-36, cardX+cardW-20, cardY+cardH-36);

    p.setFont(QFont("Segoe UI", 8));
    p.setPen(QColor(71,85,105, static_cast<int>(180 * m_opacity)));
    p.drawText(QRectF(cardX+20, cardY+cardH-30, cardW-40, 24),
               Qt::AlignCenter,
               "Switch-Case Compiler  •  Educational Tool  •  Ctrl+/ to toggle");
}

void ShortcutOverlay::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Escape ||
        (e->key() == Qt::Key_Slash && e->modifiers() & Qt::ControlModifier) ||
        (e->key() == Qt::Key_Question && e->modifiers() & Qt::ControlModifier)) {
        toggleVisible();
    }
}

void ShortcutOverlay::mousePressEvent(QMouseEvent*) {
    toggleVisible();
}
