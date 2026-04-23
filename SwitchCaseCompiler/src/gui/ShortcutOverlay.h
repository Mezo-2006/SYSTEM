#pragma once
#include <QWidget>
#include <QPropertyAnimation>

class ShortcutOverlay : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    explicit ShortcutOverlay(QWidget* parent = nullptr);
    void toggleVisible();

    qreal opacity() const { return m_opacity; }
    void  setOpacity(qreal v);

protected:
    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void mousePressEvent(QMouseEvent*) override;

private:
    qreal m_opacity = 0.0;
    QPropertyAnimation* m_anim;
    bool m_showing = false;
};
