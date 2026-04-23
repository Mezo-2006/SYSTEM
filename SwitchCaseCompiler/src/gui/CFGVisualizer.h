#ifndef CFGVISUALIZER_H
#define CFGVISUALIZER_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <vector>
#include <string>
#include "../core/TACGenerator.h"

class CFGVisualizer : public QGraphicsView {
    Q_OBJECT

public:
    explicit CFGVisualizer(QWidget* parent = nullptr);
    void buildGraph(const std::vector<BasicBlock>& basicBlocks);

protected:
    void wheelEvent(QWheelEvent* event) override;

private:
    QGraphicsScene* scene;
};

#endif // CFGVISUALIZER_H
