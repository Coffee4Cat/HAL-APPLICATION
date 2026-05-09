#pragma once

#include <QWidget>
#include <QPointF>
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>


class JoystickWidget : public QWidget {
    Q_OBJECT

public:
    explicit JoystickWidget(QWidget *parent = nullptr);

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

    const qreal radius = 180;
    const qreal handleRadius = 40;
};