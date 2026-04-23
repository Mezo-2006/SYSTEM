#ifndef AST_VIEW_H
#define AST_VIEW_H

#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>
#include <QTimer>
#include <QPointF>
#include <QColor>
#include <QFont>
#include <vector>
#include <memory>
#include <functional>
#include "../core/AST.h"

// ─────────────────────────────────────────────────────────────────────────────
//  ASTView  –  Beautiful, animated, interactive AST visualizer
//
//  Node colour scheme (VS-Code-inspired):
//    Program / SwitchStatement  →  purple  (#7C3AED)
//    CaseClause / default       →  amber   (#D97706)
//    AssignmentStatement        →  blue    (#2563EB)
//    DeclarationStatement       →  teal    (#0D9488)
//    BinaryExpression           →  rose    (#E11D48)
//    Identifier                 →  cyan    (#0891B2)
//    Constant                   →  green   (#16A34A)
//    StringLiteral              →  orange  (#EA580C)
//    Cin / Cout                 →  indigo  (#4F46E5)
// ─────────────────────────────────────────────────────────────────────────────

struct ASTViewNode {
    // Layout
    qreal targetX = 0, targetY = 0;
    qreal currentX = 0, currentY = 0;
    qreal scale = 0.0, opacity = 0.0;
    int   depth = 0;
    bool  isHovered = false;

    // Content
    QString label;       // e.g. "BinaryExpr (+)"
    QString sublabel;    // e.g. "line 5"
    QColor  color;
    QColor  borderColor;
    bool    isLeaf = false;

    // Connectivity
    ASTNode*            astNode  = nullptr;
    std::vector<int>    childIndices;   // indices into the flat nodePositions vector
    int                 parentIndex = -1;
};

class ASTView : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal animProgress READ animProgress WRITE setAnimProgress)

public:
    explicit ASTView(QWidget* parent = nullptr);
    ~ASTView() override;

    void setAST(ASTNode* root);
    void zoomIn();
    void zoomOut();
    void resetView();
    int  zoomPercent() const { return static_cast<int>(m_zoom * 100.0); }

    qreal animProgress() const { return m_animProgress; }
    void  setAnimProgress(qreal v);

signals:
    void nodeClicked(ASTNode* node);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;

private:
    // ── tree building ──────────────────────────────────────────────────────
    void buildFromAST(ASTNode* root);
    int  addNode(const QString& label, const QString& sublabel,
                 const QColor& color, ASTNode* astNode,
                 bool isLeaf, int parentIdx);

    // visitor helpers
    int  visitNode(ASTNode* node, int parentIdx);
    int  visitProgram(Program* n, int p);
    int  visitSwitch(SwitchStatement* n, int p);
    int  visitCase(CaseClause* n, int p);
    int  visitAssign(AssignmentStatement* n, int p);
    int  visitDecl(DeclarationStatement* n, int p);
    int  visitCin(CinStatement* n, int p);
    int  visitCout(CoutStatement* n, int p);
    int  visitBinary(BinaryExpression* n, int p);
    int  visitIdentifier(Identifier* n, int p);
    int  visitConstant(Constant* n, int p);
    int  visitStringLit(StringLiteral* n, int p);

    // ── layout ────────────────────────────────────────────────────────────
    qreal subtreeWidth(int idx) const;
    qreal nodeWidth(int idx) const;
    void  layoutSubtree(int idx, qreal x, qreal y, qreal& nextX);

    // ── drawing ───────────────────────────────────────────────────────────
    void drawBackground(QPainter& p);
    void drawEdge(QPainter& p, int parentIdx, int childIdx);
    void drawNode(QPainter& p, int idx);
    void drawEmptyState(QPainter& p);

    // ── animation ─────────────────────────────────────────────────────────
    void startAnimation();
    void interpolate(qreal progress);

    // ── data ──────────────────────────────────────────────────────────────
    std::vector<ASTViewNode> m_nodes;
    ASTNode*                 m_root = nullptr;

    QPropertyAnimation* m_anim  = nullptr;
    qreal               m_animProgress = 0.0;

    qreal   m_zoom = 1.0;
    QPointF m_pan  = {60.0, 60.0};
    QPoint  m_lastPan;
    bool    m_panning = false;
    int     m_hoveredIdx = -1;

    static constexpr qreal V_SPACING = 110.0;
    static constexpr qreal H_GAP     = 28.0;
    static constexpr qreal NODE_H    = 52.0;
};

#endif // AST_VIEW_H
