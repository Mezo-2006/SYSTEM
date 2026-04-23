#include "ParseTreeView.h"
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFontMetrics>
#include <cmath>

ParseTreeView::ParseTreeView(QWidget* parent) 
    : QWidget(parent), rootNode(nullptr), m_animationProgress(0.0), 
    hoveredNodeIndex(-1), zoomLevel(1.0), panOffset(40.0, 40.0), isPanning(false) {
    
    setMinimumSize(800, 600);
    setMouseTracking(true);  // Enable hover detection
    
    setStyleSheet("background: #1e1e1e;");
    
    // Setup animation
    buildAnimation = new QPropertyAnimation(this, "animationProgress");
    buildAnimation->setDuration(1500);  // 1.5 seconds
    buildAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, [this]() {
        update();
    });
}

ParseTreeView::~ParseTreeView() {
    delete buildAnimation;
    delete animationTimer;
}

void ParseTreeView::zoomIn() {
    zoomLevel = qBound(0.45, zoomLevel + 0.1, 2.4);
    update();
}

void ParseTreeView::zoomOut() {
    zoomLevel = qBound(0.45, zoomLevel - 0.1, 2.4);
    update();
}

void ParseTreeView::resetView() {
    zoomLevel = 1.0;
    panOffset = QPointF(40.0, 40.0);
    if (!nodePositions.empty()) {
        qreal minX = 1e9, maxX = 0;
        for (const auto& pos : nodePositions) {
            minX = std::min(minX, pos.targetX);
            maxX = std::max(maxX, pos.targetX);
        }
        const qreal treeCenterX = (minX + maxX) * 0.5;
        panOffset = QPointF(width() * 0.5 - treeCenterX, 48.0);
    }
    update();
}

void ParseTreeView::setAnimationProgress(qreal progress) {
    m_animationProgress = progress;
    interpolatePositions(progress);
    update();
}

qreal ParseTreeView::calculateSubtreeWidth(ParseTreeNode* node) {
    if (!node) return 0;
    
    if (node->children.empty()) {
        return nodeVisualWidth(node) + 36.0;
    }
    
    qreal totalWidth = 0;
    for (const auto& child : node->children) {
        totalWidth += calculateSubtreeWidth(child.get());
    }
    
    return std::max(nodeVisualWidth(node) + 20.0, totalWidth);
}

qreal ParseTreeView::nodeVisualWidth(const ParseTreeNode* node) const {
    if (!node) {
        return 120.0;
    }

    const int labelLen = static_cast<int>(node->label.size());
    const qreal base = node->isTerminal ? 112.0 : 126.0;
    return qBound(112.0, base + labelLen * 5.2, 260.0);
}

void ParseTreeView::calculatePositions(ParseTreeNode* node, qreal x, qreal y, 
                                       int depth, qreal& nextX, qreal width) {
    if (!node) return;
    
    const qreal VERTICAL_SPACING = 108;
    const qreal MIN_HORIZONTAL_SPACING = 30;
    
    if (node->children.empty()) {
        // Leaf node
        NodePosition pos;
        pos.targetX = nextX;
        pos.targetY = y;
        pos.currentX = nextX;
        pos.currentY = -50;  // Start from above for animation
        pos.x = nextX;
        pos.y = y;
        pos.node = node;
        pos.scale = 0.0;
        pos.opacity = 0.0;
        pos.depth = depth;
        pos.isHovered = false;
        
        nodePositions.push_back(pos);
        nextX += std::max(nodeVisualWidth(node), width) + MIN_HORIZONTAL_SPACING;
    } else {
        // Internal node - position after laying out children
        qreal childStartX = nextX;
        
        for (const auto& child : node->children) {
            qreal childWidth = calculateSubtreeWidth(child.get());
            calculatePositions(child.get(), nextX, y + VERTICAL_SPACING, 
                             depth + 1, nextX, childWidth);
        }
        
        qreal childEndX = nextX;
        qreal nodeX = (childStartX + childEndX - MIN_HORIZONTAL_SPACING) / 2.0;
        
        NodePosition pos;
        pos.targetX = nodeX;
        pos.targetY = y;
        pos.currentX = nodeX;
        pos.currentY = -50;  // Start from above for animation
        pos.x = nodeX;
        pos.y = y;
        pos.node = node;
        pos.scale = 0.0;
        pos.opacity = 0.0;
        pos.depth = depth;
        pos.isHovered = false;
        
        nodePositions.push_back(pos);
    }
}

void ParseTreeView::setParseTree(ParseTreeNode* root) {
    rootNode = root;
    nodePositions.clear();
    hoveredNodeIndex = -1;
    
    if (rootNode) {
        qreal nextX = 100;
        qreal rootWidth = calculateSubtreeWidth(rootNode);
        calculatePositions(rootNode, 50, 80, 0, nextX, rootWidth);
        
        // Calculate widget size
        qreal maxX = 0, maxY = 0, minX = 1e9;
        for (const auto& pos : nodePositions) {
            maxX = std::max(maxX, pos.targetX);
            maxY = std::max(maxY, pos.targetY);
            minX = std::min(minX, pos.targetX);
        }
        setMinimumSize(static_cast<int>(maxX + 200), static_cast<int>(maxY + 220));

        const qreal treeCenterX = (minX + maxX) * 0.5;
        panOffset = QPointF(width() * 0.5 - (treeCenterX * zoomLevel), 48.0);
        
        // Start animation
        startBuildAnimation();
    }
    
    update();
}

void ParseTreeView::startBuildAnimation() {
    buildAnimation->stop();
    buildAnimation->setStartValue(0.0);
    buildAnimation->setEndValue(1.0);
    buildAnimation->start();
}

void ParseTreeView::interpolatePositions(qreal progress) {
    for (auto& pos : nodePositions) {
        // Animate position
        pos.currentX = pos.targetX;
        pos.currentY = pos.currentY + (pos.targetY - pos.currentY) * progress;
        
        // Animate scale and opacity
        pos.scale = progress;
        pos.opacity = progress;
    }
}

QColor ParseTreeView::getNodeColor(const ParseTreeNode* node, bool isHovered) {
    if (isHovered) {
        return node->isTerminal ? QColor(86, 212, 160) : QColor(86, 156, 214);
    }
    
    if (node->isTerminal) {
        return QColor(78, 201, 176); // VS Code terminal color (teal)
    } else {
        return QColor(0, 122, 204); // VS Code non-terminal color (blue)
    }
}

QColor ParseTreeView::getEdgeColor(int depth) {
    return QColor(100, 100, 100);
}

void ParseTreeView::drawBackground(QPainter& painter) {
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor(30, 30, 30));
    bg.setColorAt(1.0, QColor(37, 37, 38));
    painter.fillRect(rect(), bg);

    painter.setPen(QPen(QColor(255, 255, 255, 10), 1));
    for (int x = 0; x < width(); x += 48) {
        painter.drawLine(x, 0, x, height());
    }
    for (int y = 0; y < height(); y += 48) {
        painter.drawLine(0, y, width(), y);
    }

    painter.setPen(QColor(150, 150, 150));
    painter.setFont(QFont("Segoe UI", 10));
    painter.drawText(16, 24, "Parse Tree View  •  Left click node  •  Middle/Right drag pan  •  Mouse wheel zoom");
}

void ParseTreeView::drawEdge(QPainter& painter, const NodePosition& parent, 
                             const NodePosition& child, bool isAnimating) {
    if (isAnimating && (parent.opacity < 0.1 || child.opacity < 0.1)) {
        return;
    }
    
    // Calculate edge points
    qreal x1 = parent.currentX;
    const qreal parentHalfHeight = 24.0 * parent.scale;
    const qreal childHalfHeight = 24.0 * child.scale;
    qreal y1 = parent.currentY + parentHalfHeight;
    qreal x2 = child.currentX;
    qreal y2 = child.currentY - childHalfHeight;
    
    // Curved edge using cubic bezier
    QPainterPath path;
    path.moveTo(x1, y1);
    
    qreal controlY = (y1 + y2) / 2;
    path.cubicTo(x1, controlY, x2, controlY, x2, y2);
    
    // Color based on depth
    QColor edgeColor = getEdgeColor(parent.depth);
    edgeColor.setAlphaF(std::min(parent.opacity, child.opacity));
    
    // Draw shadow
    painter.setPen(QPen(QColor(0, 0, 0, 35), 3));
    painter.drawPath(path.translated(2, 2));
    
    // Draw edge with gradient
    QLinearGradient gradient(x1, y1, x2, y2);
    gradient.setColorAt(0, edgeColor);
    gradient.setColorAt(1, edgeColor.darker(120));
    
    painter.setPen(QPen(QBrush(gradient), 2.1));
    painter.drawPath(path);
    
    // Draw arrow head
    if (child.opacity > 0.5) {
        qreal angle = std::atan2(y2 - y1, x2 - x1);
        qreal arrowSize = 8;
        
        QPointF arrowP1 = QPointF(x2 - arrowSize * std::cos(angle - M_PI / 6),
                                  y2 - arrowSize * std::sin(angle - M_PI / 6));
        QPointF arrowP2 = QPointF(x2 - arrowSize * std::cos(angle + M_PI / 6),
                                  y2 - arrowSize * std::sin(angle + M_PI / 6));
        
        QPolygonF arrowHead;
        arrowHead << QPointF(x2, y2) << arrowP1 << arrowP2;
        
        painter.setBrush(edgeColor);
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(arrowHead);
    }
}

void ParseTreeView::drawNode(QPainter& painter, const NodePosition& pos, bool isAnimating) {
    if (isAnimating && pos.opacity < 0.01) {
        return;
    }
    
    const qreal cardWidth = nodeVisualWidth(pos.node) * pos.scale;
    const qreal cardHeight = 48.0 * pos.scale;
    const qreal corner = 12.0 * pos.scale;
    
    qreal x = pos.currentX;
    qreal y = pos.currentY;
    
    // Draw shadow
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 40 * pos.opacity));
    QRectF shadowRect(x - cardWidth / 2 + 2.5, y - cardHeight / 2 + 2.5, cardWidth, cardHeight);
    painter.drawRoundedRect(shadowRect, corner, corner);
    
    // Draw node with gradient
    QLinearGradient gradient(x - cardWidth / 2, y - cardHeight / 2, x + cardWidth / 2, y + cardHeight / 2);
    QColor baseColor = getNodeColor(pos.node, pos.isHovered);
    baseColor.setAlphaF(pos.opacity);
    
    gradient.setColorAt(0, baseColor.lighter(120));
    gradient.setColorAt(1, baseColor.darker(112));
    
    painter.setBrush(gradient);
    
    // Border color
    QColor borderColor = pos.node->isTerminal ? QColor(78, 201, 176) : QColor(0, 122, 204);
    borderColor.setAlphaF(pos.opacity);
    
    if (pos.isHovered) {
        painter.setPen(QPen(QColor(255, 255, 255, 200), 2));
    } else {
        painter.setPen(QPen(borderColor, 1));
    }
    
    QRectF cardRect(x - cardWidth / 2, y - cardHeight / 2, cardWidth, cardHeight);
    painter.drawRoundedRect(cardRect, corner, corner);
    
    // Draw label with proper font
    if (pos.scale > 0.3) {
        painter.setPen(QColor(255, 255, 255, 255 * pos.opacity));
        QFont font = painter.font();
        font.setPointSize(10);
        font.setBold(!pos.node->isTerminal);
        painter.setFont(font);
        
        QString label = QString::fromStdString(pos.node->label);
        
        // Smart truncation
        if (label.length() > 24) {
            label = label.left(22) + "..";
        }
        
        QRectF textRect(x - cardWidth / 2 + 6, y - 13, cardWidth - 12, 20);
        painter.drawText(textRect, Qt::AlignCenter, label);
        
        // Draw type indicator
        if (pos.scale > 0.7) {
            font.setPointSize(7);
            font.setBold(false);
            font.setItalic(true);
            painter.setFont(font);
            
            QString type = pos.node->isTerminal ? "terminal" : "non-terminal";
            painter.setPen(QColor(220, 220, 220, 180 * pos.opacity));
            QRectF typeRect(x - cardWidth / 2 + 6, y + 8, cardWidth - 12, 14);
            painter.drawText(typeRect, Qt::AlignCenter, type);
        }
    }
}

void ParseTreeView::drawTree(QPainter& painter) {
    if (!rootNode || nodePositions.empty()) {
        // Draw empty state message
        painter.setPen(QColor(193, 211, 235));
        QFont font = painter.font();
        font.setPointSize(14);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, 
                        "No Parse Tree Available\n\nCompile your code to visualize the grammar structure");
        return;
    }
    
    bool isAnimating = (m_animationProgress < 1.0);
    
    // Draw edges first (back to front)
    for (const auto& pos : nodePositions) {
        for (const auto& child : pos.node->children) {
            // Find child position
            for (const auto& childPos : nodePositions) {
                if (childPos.node == child.get()) {
                    drawEdge(painter, pos, childPos, isAnimating);
                    break;
                }
            }
        }
    }
    
    // Draw nodes (front)
    for (const auto& pos : nodePositions) {
        drawNode(painter, pos, isAnimating);
    }
    
    // Draw info panel
    if (m_animationProgress > 0.9) {
        painter.setPen(QColor(200, 200, 200));
        QFont font = painter.font();
        font.setPointSize(9);
        painter.setFont(font);
        
        QString info = QString("Nodes: %1 | Depth: %2 | Zoom: %3%")
                          .arg(nodePositions.size())
                          .arg(nodePositions.empty() ? 0 : 
                               (*std::max_element(nodePositions.begin(), nodePositions.end(),
                                   [](const NodePosition& a, const NodePosition& b) {
                                       return a.depth < b.depth;
                                   })).depth + 1)
                          .arg(static_cast<int>(zoomLevel * 100));
        
        const int hudY = static_cast<int>((height() / zoomLevel) - 28);
        painter.drawText(12, hudY, info);
        painter.setPen(QColor(150, 150, 150));
        painter.drawText(12, hudY + 14, "NT: blue  |  T: teal");
    }
}

void ParseTreeView::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::TextAntialiasing);
    
    drawBackground(painter);
    
    painter.translate(panOffset);
    painter.scale(zoomLevel, zoomLevel);
    
    drawTree(painter);
}

void ParseTreeView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton || event->button() == Qt::RightButton) {
        isPanning = true;
        lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }

    // Check if clicked on a node
    qreal clickX = (event->pos().x() - panOffset.x()) / zoomLevel;
    qreal clickY = (event->pos().y() - panOffset.y()) / zoomLevel;
    
    for (const auto& pos : nodePositions) {
        qreal dx = clickX - pos.currentX;
        qreal dy = clickY - pos.currentY;
        qreal halfW = (nodeVisualWidth(pos.node) * pos.scale) / 2.0;
        qreal halfH = (48.0 * pos.scale) / 2.0;
        
        if (std::abs(dx) <= halfW && std::abs(dy) <= halfH) {
            if (pos.node->derivationStep >= 0) {
                emit nodeClicked(pos.node->derivationStep);
            }
            if (pos.node && pos.node->isTerminal) {
                const QString lexeme = QString::fromStdString(pos.node->label).trimmed();
                if (!lexeme.isEmpty() && lexeme != "ε") {
                    const QString tip = QString("Parse tree terminal: %1").arg(lexeme);
                    emit nodeLexemeRequested(lexeme, tip);
                }
            }
            break;
        }
    }
}

void ParseTreeView::mouseMoveEvent(QMouseEvent* event) {
    if (isPanning) {
        QPoint delta = event->pos() - lastPanPoint;
        panOffset += QPointF(delta.x(), delta.y());
        lastPanPoint = event->pos();
        update();
        return;
    }

    // Update hover state
    qreal mouseX = (event->pos().x() - panOffset.x()) / zoomLevel;
    qreal mouseY = (event->pos().y() - panOffset.y()) / zoomLevel;
    
    bool needsUpdate = false;
    hoveredNodeIndex = -1;
    
    for (size_t i = 0; i < nodePositions.size(); ++i) {
        auto& pos = nodePositions[i];
        qreal dx = mouseX - pos.currentX;
        qreal dy = mouseY - pos.currentY;
        qreal halfW = (nodeVisualWidth(pos.node) * pos.scale) / 2.0;
        qreal halfH = (48.0 * pos.scale) / 2.0;
        
        bool wasHovered = pos.isHovered;
        pos.isHovered = (std::abs(dx) <= halfW && std::abs(dy) <= halfH);
        
        if (pos.isHovered) {
            hoveredNodeIndex = i;
            setCursor(Qt::PointingHandCursor);
        }
        
        if (wasHovered != pos.isHovered) {
            needsUpdate = true;
        }
    }
    
    if (hoveredNodeIndex == -1) {
        setCursor(Qt::ArrowCursor);
    }
    
    if (needsUpdate) {
        update();
    }
}

void ParseTreeView::mouseReleaseEvent(QMouseEvent* event) {
    if (isPanning && (event->button() == Qt::MiddleButton || event->button() == Qt::RightButton)) {
        isPanning = false;
        setCursor(Qt::ArrowCursor);
    }
}

void ParseTreeView::wheelEvent(QWheelEvent* event) {
    const qreal oldZoom = zoomLevel;
    qreal delta = event->angleDelta().y() / 1200.0;
    zoomLevel = qBound(0.45, zoomLevel + delta, 2.4);

    QPointF mousePos = event->position();
    QPointF sceneBefore = (mousePos - panOffset) / oldZoom;
    QPointF sceneAfter = sceneBefore * zoomLevel;
    panOffset += mousePos - (sceneAfter + panOffset);
    
    update();
}

