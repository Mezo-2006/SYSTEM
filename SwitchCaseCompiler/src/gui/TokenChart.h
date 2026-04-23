#pragma once
#include <QWidget>
#include <vector>
#include <map>
#include <string>
#include "../core/Lexer.h"

class TokenChart : public QWidget {
    Q_OBJECT
public:
    explicit TokenChart(QWidget* parent = nullptr);
    void setTokens(const std::vector<Token>& tokens);
    void reset();

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;

private:
    struct Bar {
        QString label;
        int     count;
        QColor  color;
        QRectF  rect;
    };
    std::vector<Bar> bars;
    int maxCount = 1;
    int hoveredIdx = -1;
    int totalTokens = 0;

    static QColor colorForType(const QString& type);
};
