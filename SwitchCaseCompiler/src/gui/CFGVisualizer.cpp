#include "CFGVisualizer.h"
#include <QGraphicsTextItem>
#include <QWheelEvent>
#include <QDebug>

CFGVisualizer::CFGVisualizer(QWidget* parent) : QGraphicsView(parent) {
    scene = new QGraphicsScene(this);
    setScene(scene);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setStyleSheet("background-color: #1e1e1e; border: none;");
}

void CFGVisualizer::buildGraph(const std::vector<BasicBlock>& basicBlocks) {
    scene->clear();
    if (basicBlocks.empty()) {
        return;
    }

    std::map<int, QGraphicsRectItem*> blockItems;
    int yPos = 0;

    for (const auto& block : basicBlocks) {
        QString blockText = QString("Block %1\n").arg(block.id);
        for (const auto& instr : block.instructions) {
            blockText += QString::fromStdString(instr.toString()) + "\n";
        }

        QGraphicsTextItem* textItem = new QGraphicsTextItem(blockText);
        textItem->setDefaultTextColor(QColor("#d4d4d4"));
        
        QGraphicsRectItem* rectItem = new QGraphicsRectItem(textItem->boundingRect());
        rectItem->setBrush(QColor("#2d2d2d"));
        rectItem->setPen(QPen(QColor("#569cd6"), 2));
        textItem->setParentItem(rectItem);

        rectItem->setPos(0, yPos);
        scene->addItem(rectItem);
        blockItems[block.id] = rectItem;

        yPos += rectItem->rect().height() + 50;
    }

    for (const auto& block : basicBlocks) {
        QGraphicsRectItem* fromItem = blockItems[block.id];
        if (block.true_exit != -1) {
            QGraphicsRectItem* toItem = blockItems[block.true_exit];
            QLineF line(fromItem->pos() + QPointF(fromItem->rect().width() / 2, fromItem->rect().height()), 
                        toItem->pos() + QPointF(toItem->rect().width() / 2, 0));
            scene->addLine(line, QPen(QColor("#4ec9b0"), 2));
        }
        if (block.false_exit != -1) {
            QGraphicsRectItem* toItem = blockItems[block.false_exit];
            QLineF line(fromItem->pos() + QPointF(fromItem->rect().width() / 2, fromItem->rect().height()), 
                        toItem->pos() + QPointF(toItem->rect().width() / 2, 0));
            scene->addLine(line, QPen(QColor("#f48771"), 2));
        }
    }
}

void CFGVisualizer::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() > 0) {
        scale(1.1, 1.1);
    } else {
        scale(0.9, 0.9);
    }
}
