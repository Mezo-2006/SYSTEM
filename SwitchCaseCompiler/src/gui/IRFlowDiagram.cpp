#include "IRFlowDiagram.h"
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

// Color scheme for different instruction types
static QColor getInstructionColor(const QString& opcode) {
    if (opcode == "=") return QColor("#4F46E5");        // Indigo for assignments
    if (opcode == "+" || opcode == "-") return QColor("#059669");  // Green for arithmetic
    if (opcode == "*" || opcode == "/") return QColor("#DC2626");  // Red for multiplication/division
    if (opcode == "LABEL") return QColor("#7C3AED");    // Purple for labels
    if (opcode == "goto") return QColor("#EA580C");     // Orange for jumps
    if (opcode == "if_goto" || opcode == "if_false_goto") return QColor("#DB2777");  // Pink for conditional jumps
    if (opcode == "==" || opcode == "!=") return QColor("#0891B2"); // Cyan for comparisons
    if (opcode == "cin" || opcode == "cout") return QColor("#0EA5E9"); // Blue for I/O
    if (opcode == "return") return QColor("#A855F7"); // Violet for returns
    return QColor("#6B7280"); // Gray for others
}

IRFlowNode::IRFlowNode(const TACInstruction& tac, int order) 
    : executionOrder(order) {
    opcode = QString::fromStdString(tac.opcodeToString());
    result = QString::fromStdString(tac.result);
    arg1 = QString::fromStdString(tac.arg1);
    arg2 = QString::fromStdString(tac.arg2);
    instruction = QString::fromStdString(tac.toString());
    color = getInstructionColor(opcode);
}

IRFlowDiagram::IRFlowDiagram(QWidget* parent) : QWidget(parent) {
    setMinimumSize(800, 600);
    setMouseTracking(true);
    setStyleSheet("background: #0f1117;");
    
    m_executionTimer = new QTimer(this);
    m_executionTimer->setInterval(800); // 800ms per step
    connect(m_executionTimer, &QTimer::timeout, this, &IRFlowDiagram::onExecutionTimer);
    
    m_animation = new QPropertyAnimation(this, "animProgress", this);
    m_animation->setDuration(600);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
}

IRFlowDiagram::~IRFlowDiagram() {}

void IRFlowDiagram::setTACInstructions(const std::vector<TACInstruction>& instructions) {
    m_nodes.clear();
    m_currentStep = -1;
    m_isExecuting = false;
    
    for (size_t i = 0; i < instructions.size(); ++i) {
        m_nodes.emplace_back(instructions[i], static_cast<int>(i));
    }
    
    layoutNodes();
    update();
}

void IRFlowDiagram::setOptimizedTAC(const std::vector<TACInstruction>& original, 
                                   const std::vector<TACInstruction>& optimized) {
    setTACInstructions(original);
    
    m_optimizedNodes.clear();
    for (size_t i = 0; i < optimized.size(); ++i) {
        m_optimizedNodes.emplace_back(optimized[i], static_cast<int>(i));
    }
    
    // Mark optimized nodes
    for (auto& node : m_optimizedNodes) {
        node.isOptimized = true;
        node.color = node.color.lighter(120);
    }

    m_showOptimized = false;
    layoutNodes();
    update();
}

void IRFlowDiagram::setShowOptimized(bool showOptimized) {
    const bool canShowOptimized = showOptimized && !m_optimizedNodes.empty();
    if (m_showOptimized == canShowOptimized) {
        return;
    }

    m_showOptimized = canShowOptimized;
    resetExecution();
    layoutNodes();
    update();
}

std::vector<IRFlowNode>& IRFlowDiagram::activeNodes() {
    if (m_showOptimized && !m_optimizedNodes.empty()) {
        return m_optimizedNodes;
    }
    return m_nodes;
}

const std::vector<IRFlowNode>& IRFlowDiagram::activeNodes() const {
    if (m_showOptimized && !m_optimizedNodes.empty()) {
        return m_optimizedNodes;
    }
    return m_nodes;
}

void IRFlowDiagram::layoutNodes() {
    auto layoutVector = [](std::vector<IRFlowNode>& nodes) {
        if (nodes.empty()) {
            return;
        }

        const int cols = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(nodes.size()))));
        for (size_t i = 0; i < nodes.size(); ++i) {
            const int row = static_cast<int>(i) / cols;
            const int col = static_cast<int>(i) % cols;
            nodes[i].position = QPointF(
                80 + col * NODE_SPACING_X,
                90 + row * NODE_SPACING_Y
            );
        }
    };

    layoutVector(m_nodes);
    layoutVector(m_optimizedNodes);
}

void IRFlowDiagram::startExecution() {
    auto& nodes = activeNodes();
    if (nodes.empty()) return;
    
    m_isExecuting = true;
    m_currentStep = 0;
    m_executionTimer->start();
    
    // Reset all highlights
    for (auto& node : m_nodes) {
        node.isHighlighted = false;
    }
    for (auto& node : m_optimizedNodes) {
        node.isHighlighted = false;
    }

    nodes[m_currentStep].isHighlighted = true;
    emit instructionHighlighted(m_currentStep);
    update();
}

void IRFlowDiagram::pauseExecution() {
    m_isExecuting = false;
    m_executionTimer->stop();
}

void IRFlowDiagram::resetExecution() {
    m_isExecuting = false;
    m_executionTimer->stop();
    m_currentStep = -1;
    
    for (auto& node : m_nodes) {
        node.isHighlighted = false;
    }
    for (auto& node : m_optimizedNodes) {
        node.isHighlighted = false;
    }
    
    update();
}

void IRFlowDiagram::stepForward() {
    auto& nodes = activeNodes();
    if (nodes.empty()) {
        return;
    }

    if (m_currentStep < static_cast<int>(nodes.size()) - 1) {
        if (m_currentStep >= 0) {
            nodes[m_currentStep].isHighlighted = false;
        }
        m_currentStep++;
        nodes[m_currentStep].isHighlighted = true;
        emit instructionHighlighted(m_currentStep);
        update();
    }
}

void IRFlowDiagram::stepBackward() {
    auto& nodes = activeNodes();
    if (nodes.empty()) {
        return;
    }

    if (m_currentStep > 0 && m_currentStep < static_cast<int>(nodes.size())) {
        nodes[m_currentStep].isHighlighted = false;
        m_currentStep--;
        nodes[m_currentStep].isHighlighted = true;
        emit instructionHighlighted(m_currentStep);
        update();
    }
}

void IRFlowDiagram::onExecutionTimer() {
    stepForward();

    const auto& nodes = activeNodes();
    if (!nodes.empty() && m_currentStep >= static_cast<int>(nodes.size()) - 1) {
        pauseExecution();
        emit executionFinished();
    }
}

void IRFlowDiagram::setAnimProgress(qreal progress) {
    m_animProgress = progress;
    update();
}

void IRFlowDiagram::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    
    drawBackground(painter);
    
    if (m_nodes.empty()) {
        painter.setPen(QColor(100, 110, 140));
        QFont font("Segoe UI", 15);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter,
                        "No IR Instructions\n\nCompile your code to see the intermediate representation flow");
        return;
    }
    
    painter.translate(m_pan);
    painter.scale(m_zoom, m_zoom);
    
    drawConnections(painter);
    
    // Draw nodes
    const auto& nodesToDraw = activeNodes();
    for (const auto& node : nodesToDraw) {
        drawNode(painter, node);
    }
    
    drawExecutionFlow(painter);
}

void IRFlowDiagram::drawBackground(QPainter& painter) {
    // Dark gradient background
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor("#0f1117"));
    bg.setColorAt(1.0, QColor("#1a1d27"));
    painter.fillRect(rect(), bg);
    
    // Grid pattern
    painter.setPen(QPen(QColor(255, 255, 255, 8), 1));
    for (int x = 0; x < width(); x += 50) {
        painter.drawLine(x, 0, x, height());
    }
    for (int y = 0; y < height(); y += 50) {
        painter.drawLine(0, y, width(), y);
    }
    
    // Header
    painter.setPen(QColor(120, 130, 160));
    QFont headerFont("Segoe UI", 10);
    painter.setFont(headerFont);
    painter.drawText(16, 22, "IR Flow Diagram  •  Scroll to zoom  •  Right-drag to pan  •  Click to step through execution");
}

void IRFlowDiagram::drawNode(QPainter& painter, const IRFlowNode& node) {
    QRectF rect(node.position.x() - NODE_WIDTH/2, node.position.y() - NODE_HEIGHT/2,
                NODE_WIDTH, NODE_HEIGHT);
    
    // Shadow
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 60));
    painter.drawRoundedRect(rect.translated(3, 3), 8, 8);
    
    // Highlight glow for current instruction
    if (node.isHighlighted) {
        QRadialGradient glow(rect.center(), NODE_WIDTH * 0.7);
        glow.setColorAt(0.0, QColor(255, 255, 0, 80));
        glow.setColorAt(1.0, QColor(255, 255, 0, 0));
        painter.setBrush(glow);
        painter.drawRoundedRect(rect.adjusted(-10, -10, 10, 10), 12, 12);
    }
    
    // Node body
    QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
    QColor baseColor = node.color;
    if (node.isOptimized) {
        baseColor = baseColor.lighter(130);
    }
    gradient.setColorAt(0.0, baseColor.lighter(120));
    gradient.setColorAt(1.0, baseColor.darker(110));
    
    painter.setBrush(gradient);
    painter.setPen(QPen(baseColor.lighter(140), node.isHighlighted ? 3 : 1));
    painter.drawRoundedRect(rect, 8, 8);
    
    // Instruction text
    painter.setPen(QColor(255, 255, 255, 240));
    QFont font("Consolas", 9);
    font.setBold(true);
    painter.setFont(font);
    
    // Opcode at top
    QRectF opcodeRect(rect.x() + 5, rect.y() + 5, rect.width() - 10, 15);
    painter.drawText(opcodeRect, Qt::AlignCenter, node.opcode);
    
    // Instruction details
    QFont detailFont("Consolas", 8);
    painter.setFont(detailFont);
    painter.setPen(QColor(220, 220, 220, 200));
    
    const QString details = node.instruction;
    
    QRectF detailRect(rect.x() + 5, rect.y() + 25, rect.width() - 10, 30);
    painter.drawText(detailRect, Qt::AlignCenter | Qt::TextWordWrap, details);
    
    // Execution order badge
    if (node.executionOrder >= 0) {
        QRectF badge(rect.right() - 20, rect.top() + 5, 15, 15);
        painter.setBrush(QColor(0, 0, 0, 120));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(badge);
        
        painter.setPen(QColor(255, 255, 255));
        QFont badgeFont("Segoe UI", 7);
        painter.setFont(badgeFont);
        painter.drawText(badge, Qt::AlignCenter, QString::number(node.executionOrder));
    }
}

void IRFlowDiagram::drawConnections(QPainter& painter) {
    const auto& nodes = activeNodes();
    if (nodes.size() < 2) return;
    
    painter.setPen(QPen(QColor(100, 120, 160, 100), 2));
    
    for (size_t i = 0; i < nodes.size() - 1; ++i) {
        QPointF start = nodes[i].position;
        QPointF end = nodes[i + 1].position;
        
        // Draw curved connection
        QPainterPath path;
        path.moveTo(start);
        
        QPointF control1(start.x() + 50, start.y());
        QPointF control2(end.x() - 50, end.y());
        path.cubicTo(control1, control2, end);
        
        painter.drawPath(path);
        
        // Arrow head
        double angle = std::atan2(end.y() - start.y(), end.x() - start.x());
        double arrowSize = 8.0;
        QPointF arrowP1(end.x() - arrowSize * std::cos(angle - M_PI/6),
                        end.y() - arrowSize * std::sin(angle - M_PI/6));
        QPointF arrowP2(end.x() - arrowSize * std::cos(angle + M_PI/6),
                        end.y() - arrowSize * std::sin(angle + M_PI/6));
        
        QPolygonF arrowHead;
        arrowHead << end << arrowP1 << arrowP2;
        painter.setBrush(QColor(100, 120, 160, 100));
        painter.drawPolygon(arrowHead);
    }
}

void IRFlowDiagram::drawExecutionFlow(QPainter& painter) {
    const auto& nodes = activeNodes();
    if (m_currentStep < 0 || m_currentStep >= static_cast<int>(nodes.size())) return;
    
    // Draw execution trail
    painter.setPen(QPen(QColor(255, 215, 0, 150), 4));
    for (int i = 0; i <= m_currentStep; ++i) {
        if (i > 0) {
            QPointF start = nodes[i-1].position;
            QPointF end = nodes[i].position;
            painter.drawLine(start, end);
        }
    }
}

void IRFlowDiagram::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        m_panning = true;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        QPointF scenePos = (event->pos() - m_pan) / m_zoom;
        int nodeIndex = getNodeAtPosition(scenePos);
        auto& nodes = activeNodes();
        
        if (nodeIndex >= 0 && nodeIndex < static_cast<int>(nodes.size())) {
            // Step to clicked instruction
            if (m_currentStep >= 0 && m_currentStep < static_cast<int>(nodes.size())) {
                nodes[m_currentStep].isHighlighted = false;
            }
            m_currentStep = nodeIndex;
            nodes[m_currentStep].isHighlighted = true;
            emit instructionHighlighted(m_currentStep);
            update();
        }
    }
}

void IRFlowDiagram::mouseMoveEvent(QMouseEvent* event) {
    if (m_panning) {
        QPoint delta = event->pos() - m_lastPanPoint;
        m_pan += QPointF(delta.x(), delta.y());
        m_lastPanPoint = event->pos();
        update();
        return;
    }
    
    // Update cursor based on hover
    QPointF scenePos = (event->pos() - m_pan) / m_zoom;
    int nodeIndex = getNodeAtPosition(scenePos);
    setCursor(nodeIndex >= 0 ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

void IRFlowDiagram::wheelEvent(QWheelEvent* event) {
    const qreal oldZoom = m_zoom;
    qreal delta = event->angleDelta().y() / 1200.0;
    m_zoom = qBound(0.3, m_zoom + delta, 3.0);
    
    QPointF mousePos = event->position();
    QPointF before = (mousePos - m_pan) / oldZoom;
    m_pan = mousePos - before * m_zoom;
    update();
}

int IRFlowDiagram::getNodeAtPosition(const QPointF& pos) const {
    const auto& nodes = activeNodes();
    for (size_t i = 0; i < nodes.size(); ++i) {
        QRectF rect(nodes[i].position.x() - NODE_WIDTH/2,
                   nodes[i].position.y() - NODE_HEIGHT/2,
                   NODE_WIDTH, NODE_HEIGHT);
        if (rect.contains(pos)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}