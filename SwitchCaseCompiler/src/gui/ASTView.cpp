#include "ASTView.h"
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFontMetrics>
#include <QApplication>
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
//  Colour palette
// ─────────────────────────────────────────────────────────────────────────────
static QColor C_PROGRAM   ("#7C3AED");
static QColor C_SWITCH    ("#6D28D9");
static QColor C_CASE      ("#D97706");
static QColor C_DEFAULT   ("#B45309");
static QColor C_ASSIGN    ("#2563EB");
static QColor C_DECL      ("#0D9488");
static QColor C_BINARY    ("#E11D48");
static QColor C_IDENT     ("#0891B2");
static QColor C_CONST     ("#16A34A");
static QColor C_STRING    ("#EA580C");
static QColor C_IO        ("#4F46E5");

// ─────────────────────────────────────────────────────────────────────────────
ASTView::ASTView(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(800, 600);
    setMouseTracking(true);
    setStyleSheet("background:#0f1117;");

    m_anim = new QPropertyAnimation(this, "animProgress", this);
    m_anim->setDuration(1200);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);
}

ASTView::~ASTView() {}

// ─────────────────────────────────────────────────────────────────────────────
//  Public API
// ─────────────────────────────────────────────────────────────────────────────
void ASTView::setAST(ASTNode* root)
{
    m_root = root;
    m_nodes.clear();
    m_hoveredIdx = -1;

    if (!root) { update(); return; }

    buildFromAST(root);

    // Layout
    if (!m_nodes.empty()) {
        qreal nextX = 80.0;
        layoutSubtree(0, 80.0, 80.0, nextX);

        // Compute bounding box
        qreal maxX = 0, maxY = 0, minX = 1e9;
        for (const auto& n : m_nodes) {
            maxX = std::max(maxX, n.targetX);
            maxY = std::max(maxY, n.targetY);
            minX = std::min(minX, n.targetX);
        }
        setMinimumSize(static_cast<int>(maxX + 200), static_cast<int>(maxY + 200));

        // Centre horizontally
        qreal treeCenterX = (minX + maxX) * 0.5;
        m_pan = QPointF(width() * 0.5 - treeCenterX * m_zoom, 60.0);
    }

    startAnimation();
    update();
}

void ASTView::zoomIn()  { m_zoom = qBound(0.3, m_zoom + 0.1, 2.5); update(); }
void ASTView::zoomOut() { m_zoom = qBound(0.3, m_zoom - 0.1, 2.5); update(); }

void ASTView::resetView()
{
    m_zoom = 1.0;
    if (!m_nodes.empty()) {
        qreal minX = 1e9, maxX = 0;
        for (const auto& n : m_nodes) { minX = std::min(minX, n.targetX); maxX = std::max(maxX, n.targetX); }
        m_pan = QPointF(width() * 0.5 - (minX + maxX) * 0.5, 60.0);
    } else {
        m_pan = {60.0, 60.0};
    }
    update();
}

void ASTView::setAnimProgress(qreal v)
{
    m_animProgress = v;
    interpolate(v);
    update();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Tree building  (visitor-style, manual dispatch)
// ─────────────────────────────────────────────────────────────────────────────
void ASTView::buildFromAST(ASTNode* root)
{
    visitNode(root, -1);
}

int ASTView::addNode(const QString& label, const QString& sublabel,
                     const QColor& color, ASTNode* astNode,
                     bool isLeaf, int parentIdx)
{
    ASTViewNode n;
    n.label      = label;
    n.sublabel   = sublabel;
    n.color      = color;
    n.borderColor= color.lighter(140);
    n.isLeaf     = isLeaf;
    n.astNode    = astNode;
    n.parentIndex= parentIdx;
    n.scale      = 0.0;
    n.opacity    = 0.0;

    int idx = static_cast<int>(m_nodes.size());
    m_nodes.push_back(n);

    if (parentIdx >= 0)
        m_nodes[parentIdx].childIndices.push_back(idx);

    return idx;
}

int ASTView::visitNode(ASTNode* node, int parentIdx)
{
    if (!node) return -1;

    if (auto* p = dynamic_cast<Program*>(node))              return visitProgram(p, parentIdx);
    if (auto* s = dynamic_cast<SwitchStatement*>(node))      return visitSwitch(s, parentIdx);
    if (auto* c = dynamic_cast<CaseClause*>(node))           return visitCase(c, parentIdx);
    if (auto* a = dynamic_cast<AssignmentStatement*>(node))  return visitAssign(a, parentIdx);
    if (auto* d = dynamic_cast<DeclarationStatement*>(node)) return visitDecl(d, parentIdx);
    if (auto* ci = dynamic_cast<CinStatement*>(node))        return visitCin(ci, parentIdx);
    if (auto* co = dynamic_cast<CoutStatement*>(node))       return visitCout(co, parentIdx);
    if (auto* b = dynamic_cast<BinaryExpression*>(node))     return visitBinary(b, parentIdx);
    if (auto* id = dynamic_cast<Identifier*>(node))          return visitIdentifier(id, parentIdx);
    if (auto* cn = dynamic_cast<Constant*>(node))            return visitConstant(cn, parentIdx);
    if (auto* sl = dynamic_cast<StringLiteral*>(node))       return visitStringLit(sl, parentIdx);
    return -1;
}

int ASTView::visitProgram(Program* n, int p)
{
    int idx = addNode("Program", QString("line %1").arg(n->line), C_PROGRAM, n, false, p);
    for (const auto& stmt : n->preSwitchStatements)
        visitNode(stmt.get(), idx);
    if (n->switchStmt)
        visitNode(n->switchStmt.get(), idx);
    return idx;
}

int ASTView::visitSwitch(SwitchStatement* n, int p)
{
    int idx = addNode("switch", QString("line %1").arg(n->line), C_SWITCH, n, false, p);
    // condition child
    if (n->condition) {
        int condIdx = addNode("condition", "", C_SWITCH.lighter(130), nullptr, false, idx);
        visitNode(n->condition.get(), condIdx);
    }
    // cases
    for (const auto& c : n->cases)
        visitNode(c.get(), idx);
    if (n->defaultCase)
        visitNode(n->defaultCase.get(), idx);
    return idx;
}

int ASTView::visitCase(CaseClause* n, int p)
{
    QString lbl = n->isDefault ? "default" : QString("case %1").arg(n->caseValue);
    QColor  col = n->isDefault ? C_DEFAULT : C_CASE;
    int idx = addNode(lbl, QString("line %1").arg(n->line), col, n, false, p);
    for (const auto& stmt : n->statements)
        visitNode(stmt.get(), idx);
    return idx;
}

int ASTView::visitAssign(AssignmentStatement* n, int p)
{
    int idx = addNode("assign", QString::fromStdString(n->variableName) + " =", C_ASSIGN, n, false, p);
    // lhs leaf
    addNode(QString::fromStdString(n->variableName), "lhs", C_IDENT, nullptr, true, idx);
    // rhs
    if (n->expression)
        visitNode(n->expression.get(), idx);
    return idx;
}

int ASTView::visitDecl(DeclarationStatement* n, int p)
{
    QString lbl = QString::fromStdString(n->variableType) + " " + QString::fromStdString(n->variableName);
    int idx = addNode("decl", lbl, C_DECL, n, false, p);
    if (n->initializer)
        visitNode(n->initializer.get(), idx);
    return idx;
}

int ASTView::visitCin(CinStatement* n, int p)
{
    int idx = addNode("cin >>", QString::fromStdString(n->variableName), C_IO, n, false, p);
    addNode(QString::fromStdString(n->variableName), "var", C_IDENT, nullptr, true, idx);
    return idx;
}

int ASTView::visitCout(CoutStatement* n, int p)
{
    int idx = addNode("cout <<", QString("line %1").arg(n->line), C_IO, n, false, p);
    if (n->expression)
        visitNode(n->expression.get(), idx);
    return idx;
}

int ASTView::visitBinary(BinaryExpression* n, int p)
{
    QString op = QString::fromStdString(n->op);
    int idx = addNode(op, "BinaryExpr", C_BINARY, n, false, p);
    if (n->left)  visitNode(n->left.get(),  idx);
    if (n->right) visitNode(n->right.get(), idx);
    return idx;
}

int ASTView::visitIdentifier(Identifier* n, int p)
{
    return addNode(QString::fromStdString(n->name), "id", C_IDENT, n, true, p);
}

int ASTView::visitConstant(Constant* n, int p)
{
    return addNode(QString::number(n->value), "int", C_CONST, n, true, p);
}

int ASTView::visitStringLit(StringLiteral* n, int p)
{
    QString val = "\"" + QString::fromStdString(n->value) + "\"";
    if (val.length() > 16) val = val.left(14) + "\"…";
    return addNode(val, "string", C_STRING, n, true, p);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Layout  (Reingold-Tilford-inspired, simple version)
// ─────────────────────────────────────────────────────────────────────────────
qreal ASTView::nodeWidth(int idx) const
{
    if (idx < 0 || idx >= (int)m_nodes.size()) return 120.0;
    const auto& n = m_nodes[idx];
    int maxLen = std::max(n.label.length(), n.sublabel.length());
    return qBound(100.0, 90.0 + maxLen * 6.5, 220.0);
}

qreal ASTView::subtreeWidth(int idx) const
{
    if (idx < 0 || idx >= (int)m_nodes.size()) return 0;
    const auto& n = m_nodes[idx];
    if (n.childIndices.empty())
        return nodeWidth(idx) + H_GAP;

    qreal total = 0;
    for (int c : n.childIndices)
        total += subtreeWidth(c);
    return std::max(nodeWidth(idx) + H_GAP, total);
}

void ASTView::layoutSubtree(int idx, qreal /*x*/, qreal y, qreal& nextX)
{
    if (idx < 0 || idx >= (int)m_nodes.size()) return;
    auto& n = m_nodes[idx];

    if (n.childIndices.empty()) {
        n.targetX = nextX + nodeWidth(idx) * 0.5;
        n.targetY = y;
        n.currentX = n.targetX;
        n.currentY = -80.0;
        nextX += nodeWidth(idx) + H_GAP;
    } else {
        qreal childStartX = nextX;
        for (int c : n.childIndices)
            layoutSubtree(c, nextX, y + V_SPACING, nextX);
        qreal childEndX = nextX;

        n.targetX = (childStartX + childEndX - H_GAP) * 0.5;
        n.targetY = y;
        n.currentX = n.targetX;
        n.currentY = -80.0;
    }
    n.depth = static_cast<int>(y / V_SPACING);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Animation
// ─────────────────────────────────────────────────────────────────────────────
void ASTView::startAnimation()
{
    m_anim->stop();
    m_anim->setStartValue(0.0);
    m_anim->setEndValue(1.0);
    m_anim->start();
}

void ASTView::interpolate(qreal progress)
{
    for (auto& n : m_nodes) {
        n.currentY  = n.currentY + (n.targetY - n.currentY) * progress;
        n.scale     = progress;
        n.opacity   = progress;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Drawing
// ─────────────────────────────────────────────────────────────────────────────
void ASTView::drawBackground(QPainter& p)
{
    // Dark gradient background
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor("#0f1117"));
    bg.setColorAt(1.0, QColor("#1a1d27"));
    p.fillRect(rect(), bg);

    // Subtle dot grid
    p.setPen(QPen(QColor(255, 255, 255, 12), 1));
    for (int gx = 0; gx < width(); gx += 40)
        for (int gy = 0; gy < height(); gy += 40)
            p.drawPoint(gx, gy);

    // Header hint
    p.setPen(QColor(120, 130, 160));
    QFont hf("Segoe UI", 9);
    p.setFont(hf);
    p.drawText(16, 22,
        "AST View  •  Scroll to zoom  •  Right-drag to pan  •  Click node for details");
}

void ASTView::drawEdge(QPainter& p, int parentIdx, int childIdx)
{
    if (parentIdx < 0 || childIdx < 0) return;
    const auto& par = m_nodes[parentIdx];
    const auto& chi = m_nodes[childIdx];

    if (par.opacity < 0.05 || chi.opacity < 0.05) return;

    qreal x1 = par.currentX, y1 = par.currentY + NODE_H * 0.5 * par.scale;
    qreal x2 = chi.currentX, y2 = chi.currentY - NODE_H * 0.5 * chi.scale;

    QPainterPath path;
    path.moveTo(x1, y1);
    qreal cy = (y1 + y2) * 0.5;
    path.cubicTo(x1, cy, x2, cy, x2, y2);

    // Glow shadow
    QPen shadowPen(QColor(0, 0, 0, 60), 4);
    p.setPen(shadowPen);
    p.drawPath(path.translated(1.5, 1.5));

    // Gradient edge
    QLinearGradient grad(x1, y1, x2, y2);
    QColor c1 = par.color; c1.setAlphaF(par.opacity * 0.85);
    QColor c2 = chi.color; c2.setAlphaF(chi.opacity * 0.85);
    grad.setColorAt(0, c1);
    grad.setColorAt(1, c2);
    p.setPen(QPen(QBrush(grad), 2.0));
    p.drawPath(path);

    // Arrow head
    if (chi.opacity > 0.4) {
        double angle = std::atan2(y2 - y1, x2 - x1);
        double as = 7.0;
        QPointF a1(x2 - as * std::cos(angle - M_PI / 6),
                   y2 - as * std::sin(angle - M_PI / 6));
        QPointF a2(x2 - as * std::cos(angle + M_PI / 6),
                   y2 - as * std::sin(angle + M_PI / 6));
        QPolygonF arrow;
        arrow << QPointF(x2, y2) << a1 << a2;
        p.setBrush(c2);
        p.setPen(Qt::NoPen);
        p.drawPolygon(arrow);
    }
}

void ASTView::drawNode(QPainter& p, int idx)
{
    if (idx < 0 || idx >= (int)m_nodes.size()) return;
    const auto& n = m_nodes[idx];
    if (n.opacity < 0.01) return;

    const qreal w  = nodeWidth(idx) * n.scale;
    const qreal h  = NODE_H * n.scale;
    const qreal r  = 12.0 * n.scale;
    const qreal cx = n.currentX;
    const qreal cy = n.currentY;

    QRectF rect(cx - w * 0.5, cy - h * 0.5, w, h);

    // ── Drop shadow ──────────────────────────────────────────────────────
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, static_cast<int>(55 * n.opacity)));
    p.drawRoundedRect(rect.translated(3, 4), r, r);

    // ── Glow ring (hovered) ──────────────────────────────────────────────
    if (n.isHovered) {
        QColor glow = n.color;
        glow.setAlpha(80);
        p.setPen(QPen(glow, 6));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(rect.adjusted(-3, -3, 3, 3), r + 3, r + 3);
    }

    // ── Node body gradient ───────────────────────────────────────────────
    QLinearGradient grad(rect.topLeft(), rect.bottomRight());
    QColor base = n.color;
    base.setAlphaF(n.opacity);
    grad.setColorAt(0.0, base.lighter(n.isHovered ? 145 : 125));
    grad.setColorAt(1.0, base.darker(n.isHovered ? 105 : 115));
    p.setBrush(grad);

    // Border
    QColor border = n.borderColor;
    border.setAlphaF(n.opacity);
    p.setPen(QPen(border, n.isHovered ? 2.0 : 1.2));
    p.drawRoundedRect(rect, r, r);

    // ── Leaf indicator (small circle top-right) ──────────────────────────
    if (n.isLeaf && n.scale > 0.5) {
        QColor dot = n.color.lighter(180);
        dot.setAlphaF(n.opacity * 0.9);
        p.setBrush(dot);
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(rect.right() - 8 * n.scale, rect.top() + 8 * n.scale),
                      4 * n.scale, 4 * n.scale);
    }

    // ── Text ─────────────────────────────────────────────────────────────
    if (n.scale > 0.25) {
        // Main label
        QFont lf("Segoe UI", 10);
        lf.setBold(!n.isLeaf);
        p.setFont(lf);
        p.setPen(QColor(255, 255, 255, static_cast<int>(240 * n.opacity)));

        QString label = n.label;
        QFontMetrics fm(lf);
        int maxW = static_cast<int>(w) - 14;
        if (fm.horizontalAdvance(label) > maxW)
            label = fm.elidedText(label, Qt::ElideRight, maxW);

        QRectF labelRect(rect.left() + 6, rect.top() + (n.sublabel.isEmpty() ? (h - 16) * 0.5 : 4),
                         w - 12, 18);
        p.drawText(labelRect, Qt::AlignCenter, label);

        // Sub-label
        if (!n.sublabel.isEmpty() && n.scale > 0.55) {
            QFont sf("Segoe UI", 7);
            sf.setItalic(true);
            p.setFont(sf);
            p.setPen(QColor(220, 220, 220, static_cast<int>(170 * n.opacity)));
            QRectF subRect(rect.left() + 6, rect.top() + h * 0.5 + 2, w - 12, 14);
            QString sub = n.sublabel;
            QFontMetrics sfm(sf);
            if (sfm.horizontalAdvance(sub) > maxW)
                sub = sfm.elidedText(sub, Qt::ElideRight, maxW);
            p.drawText(subRect, Qt::AlignCenter, sub);
        }
    }
}

void ASTView::drawEmptyState(QPainter& p)
{
    p.setPen(QColor(100, 110, 140));
    QFont f("Segoe UI", 15);
    f.setBold(true);
    p.setFont(f);
    p.drawText(rect(), Qt::AlignCenter,
               "No AST Available\n\nCompile your code to visualize the Abstract Syntax Tree");
}

void ASTView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    drawBackground(p);

    if (m_nodes.empty()) { drawEmptyState(p); return; }

    p.translate(m_pan);
    p.scale(m_zoom, m_zoom);

    // Edges first
    for (int i = 0; i < (int)m_nodes.size(); ++i)
        for (int c : m_nodes[i].childIndices)
            drawEdge(p, i, c);

    // Nodes on top
    for (int i = 0; i < (int)m_nodes.size(); ++i)
        drawNode(p, i);

    // HUD
    if (m_animProgress > 0.85) {
        p.resetTransform();
        p.setPen(QColor(140, 150, 180));
        QFont hf("Segoe UI", 9);
        p.setFont(hf);
        int maxDepth = 0;
        for (const auto& n : m_nodes) maxDepth = std::max(maxDepth, n.depth);
        QString info = QString("Nodes: %1  |  Depth: %2  |  Zoom: %3%")
                           .arg(m_nodes.size()).arg(maxDepth + 1)
                           .arg(static_cast<int>(m_zoom * 100));
        p.drawText(16, height() - 12, info);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Mouse events
// ─────────────────────────────────────────────────────────────────────────────
void ASTView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton) {
        m_panning  = true;
        m_lastPan  = event->pos();
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    // Hit-test
    qreal mx = (event->pos().x() - m_pan.x()) / m_zoom;
    qreal my = (event->pos().y() - m_pan.y()) / m_zoom;

    for (int i = 0; i < (int)m_nodes.size(); ++i) {
        const auto& n = m_nodes[i];
        qreal hw = nodeWidth(i) * n.scale * 0.5;
        qreal hh = NODE_H * n.scale * 0.5;
        if (std::abs(mx - n.currentX) <= hw && std::abs(my - n.currentY) <= hh) {
            if (n.astNode) emit nodeClicked(n.astNode);
            break;
        }
    }
}

void ASTView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_panning) {
        QPoint delta = event->pos() - m_lastPan;
        m_pan += QPointF(delta.x(), delta.y());
        m_lastPan = event->pos();
        update();
        return;
    }

    qreal mx = (event->pos().x() - m_pan.x()) / m_zoom;
    qreal my = (event->pos().y() - m_pan.y()) / m_zoom;

    bool changed = false;
    m_hoveredIdx = -1;

    for (int i = 0; i < (int)m_nodes.size(); ++i) {
        auto& n = m_nodes[i];
        qreal hw = nodeWidth(i) * n.scale * 0.5;
        qreal hh = NODE_H * n.scale * 0.5;
        bool hit = (std::abs(mx - n.currentX) <= hw && std::abs(my - n.currentY) <= hh);
        if (hit) m_hoveredIdx = i;
        if (n.isHovered != hit) { n.isHovered = hit; changed = true; }
    }

    setCursor(m_hoveredIdx >= 0 ? Qt::PointingHandCursor : Qt::ArrowCursor);
    if (changed) update();
}

void ASTView::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_panning && (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton)) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
    }
}

void ASTView::wheelEvent(QWheelEvent* event)
{
    const qreal oldZoom = m_zoom;
    qreal delta = event->angleDelta().y() / 1200.0;
    m_zoom = qBound(0.3, m_zoom + delta, 2.5);

    QPointF mp = event->position();
    QPointF before = (mp - m_pan) / oldZoom;
    m_pan = mp - before * m_zoom;
    update();
}
