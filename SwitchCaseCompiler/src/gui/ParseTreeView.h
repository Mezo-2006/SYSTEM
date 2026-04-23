#ifndef PARSE_TREE_VIEW_H
#define PARSE_TREE_VIEW_H

#include <QWidget>
#include <QPainter>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <QTimer>
#include <QEasingCurve>
#include <QPoint>
#include <QPointF>
#include "../core/Parser.h"

class ParseTreeView : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal animationProgress READ animationProgress WRITE setAnimationProgress)
    
private:
    ParseTreeNode* rootNode;
    QScrollArea* scrollArea;
    
    struct NodePosition {
        qreal x, y;
        qreal targetX, targetY;
        qreal currentX, currentY;
        ParseTreeNode* node;
        qreal scale;
        qreal opacity;
        int depth;
        bool isHovered;
        // NEW: track source location for interactivity
        int sourceStartPos = -1;
        int sourceEndPos = -1;
    };
    
    std::vector<NodePosition> nodePositions;
    QPropertyAnimation* buildAnimation;
    QTimer* animationTimer;
    qreal m_animationProgress;
    int hoveredNodeIndex;
    qreal zoomLevel;
    QPointF panOffset;
    QPoint lastPanPoint;
    bool isPanning;
    
    // Tree layout algorithm (improved)
    void calculatePositions(ParseTreeNode* node, qreal x, qreal y, int depth, qreal& nextX, qreal width);
    qreal calculateSubtreeWidth(ParseTreeNode* node);
    qreal nodeVisualWidth(const ParseTreeNode* node) const;
    
    // NEW: Calculate source code positions for nodes
    void calculateSourcePositions(ParseTreeNode* node, QString& sourceText);
    int getSourcePositionForNode(const NodePosition& pos);
    
    // Drawing methods
    void drawNode(QPainter& painter, const NodePosition& pos, bool isAnimating);
    void drawEdge(QPainter& painter, const NodePosition& parent, const NodePosition& child, bool isAnimating);
    void drawTree(QPainter& painter);
    void drawBackground(QPainter& painter);
    
    // Animation helpers
    void startBuildAnimation();
    void interpolatePositions(qreal progress);
    
    // Styling
    QColor getNodeColor(const ParseTreeNode* node, bool isHovered);
    QColor getEdgeColor(int depth);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    
public:
    explicit ParseTreeView(QWidget* parent = nullptr);
    ~ParseTreeView();
    
    void setParseTree(ParseTreeNode* root);
    void zoomIn();
    void zoomOut();
    void resetView();
    int zoomPercent() const { return static_cast<int>(zoomLevel * 100.0); }
    
    qreal animationProgress() const { return m_animationProgress; }
    void setAnimationProgress(qreal progress);
    
signals:
    void nodeClicked(int derivationStep);
    void nodeHighlightRequested(int line, int column, int endLine, int endColumn, const QString& tooltip);
    void nodeLexemeRequested(const QString& lexeme, const QString& tooltip);
};

#endif // PARSE_TREE_VIEW_H
