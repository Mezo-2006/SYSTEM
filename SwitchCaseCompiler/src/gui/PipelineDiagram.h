#pragma once
#include <QWidget>
#include <QPropertyAnimation>
#include <QTimer>
#include <QLabel>
#include <vector>
#include <string>

struct PhaseInfo {
    QString name;
    QString icon;
    QString color;
    qint64  durationMs = -1;
    QString status;   // "pending" | "active" | "done" | "failed"
    int     outputCount = 0;
    QString outputUnit;
};

class PipelineDiagram : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal particleProgress READ particleProgress WRITE setParticleProgress)
public:
    explicit PipelineDiagram(QWidget* parent = nullptr);

    void setPhaseStatus(int idx, const QString& status, qint64 ms, int outputCount = 0);
    void resetAll();

    qreal particleProgress() const { return m_particle; }
    void  setParticleProgress(qreal v);

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private:
    void startParticle(int fromPhase);
    void drawPhaseNode(QPainter& p, int idx, const QRectF& box);
    void drawArrow(QPainter& p, const QRectF& from, const QRectF& to, qreal progress, const QColor& col);
    void drawParticle(QPainter& p);

    std::vector<PhaseInfo> phases;
    std::vector<QRectF>    boxes;

    // particle animation
    qreal  m_particle = 0.0;
    int    m_particleFrom = -1;
    QPropertyAnimation* m_anim = nullptr;
    QTimer* m_ticker = nullptr;

    // arrow progress per segment [0..1]
    std::vector<qreal> arrowProgress;
};
