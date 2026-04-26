#ifndef IR_FLOW_DIAGRAM_H
#define IR_FLOW_DIAGRAM_H

#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>
#include <QTimer>
#include <vector>
#include <memory>
#include "../core/TACGenerator.h"

// IR Flow Node for visualization
struct IRFlowNode {
    QString instruction;
    QString opcode;
    QString result;
    QString arg1;
    QString arg2;
    QPointF position;
    QColor color;
    bool isHighlighted = false;
    bool isOptimized = false;
    int executionOrder = -1;
    
    IRFlowNode(const TACInstruction& tac, int order = -1);
};

// IR Flow Diagram - Interactive visualization of TAC execution flow
class IRFlowDiagram : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal animProgress READ animProgress WRITE setAnimProgress)

public:
    explicit IRFlowDiagram(QWidget* parent = nullptr);
    ~IRFlowDiagram() override;

    void setTACInstructions(const std::vector<TACInstruction>& instructions);
    void setOptimizedTAC(const std::vector<TACInstruction>& original, 
                        const std::vector<TACInstruction>& optimized);
    void setShowOptimized(bool showOptimized);
    bool isShowingOptimized() const { return m_showOptimized && !m_optimizedNodes.empty(); }
    void startExecution();
    void pauseExecution();
    void resetExecution();
    void stepForward();
    void stepBackward();

    qreal animProgress() const { return m_animProgress; }
    void setAnimProgress(qreal progress);

signals:
    void instructionHighlighted(int index);
    void executionFinished();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private slots:
    void onExecutionTimer();

private:
    void layoutNodes();
    void drawBackground(QPainter& painter);
    void drawNode(QPainter& painter, const IRFlowNode& node);
    void drawConnections(QPainter& painter);
    void drawExecutionFlow(QPainter& painter);
    std::vector<IRFlowNode>& activeNodes();
    const std::vector<IRFlowNode>& activeNodes() const;
    
    QPointF getNodePosition(int index) const;
    int getNodeAtPosition(const QPointF& pos) const;
    
    std::vector<IRFlowNode> m_nodes;
    std::vector<IRFlowNode> m_optimizedNodes;
    
    QTimer* m_executionTimer;
    QPropertyAnimation* m_animation;
    
    int m_currentStep = -1;
    bool m_isExecuting = false;
    bool m_showOptimized = false;
    
    qreal m_animProgress = 0.0;
    qreal m_zoom = 1.0;
    QPointF m_pan = {0, 0};
    QPoint m_lastPanPoint;
    bool m_panning = false;
    
    static constexpr qreal NODE_WIDTH = 200.0;
    static constexpr qreal NODE_HEIGHT = 60.0;
    static constexpr qreal NODE_SPACING_X = 250.0;
    static constexpr qreal NODE_SPACING_Y = 100.0;
};

#endif // IR_FLOW_DIAGRAM_H