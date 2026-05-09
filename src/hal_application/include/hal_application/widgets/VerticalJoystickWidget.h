#pragma once

#include <QWidget>
#include <QPointF>
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>
#include <QTimer>
#include <atomic>


class VerticalJoystickWidget : public QWidget {
    Q_OBJECT

public:
    explicit VerticalJoystickWidget(QWidget *parent = nullptr);

    QPointF getNormalizedVector() const;

signals:
    void joystickMoved(QPointF normalizedVector);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    
    void updateHandle(const QPointF &pos);

    bool dragging;
    QPointF handlePos;
    QTimer updateTimer;
    std::atomic<bool> update_bool{false};

    const qreal handleRadius = 40;
    qreal w = 80;
    qreal h = 200;
};