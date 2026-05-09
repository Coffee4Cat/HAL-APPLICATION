#include "widgets/HorizontalJoystickWidget.h"


HorizontalJoystickWidget::HorizontalJoystickWidget(QWidget *parent): QWidget(parent), dragging(false), handlePos(0, 0) {
    setMinimumSize(w, h);
    updateTimer.setInterval(100);
    connect(&updateTimer, &QTimer::timeout, this, [this]() {
        constexpr qreal EPSILON = 1e-5;
        if (update_bool.load()) {
            emit joystickMoved(getNormalizedVector());
            if (std::abs(handlePos.x()) < EPSILON) {
                update_bool.store(false);
            }
        }
    });
    updateTimer.start();
}

QPointF HorizontalJoystickWidget::getNormalizedVector() const {
    return handlePos / (w - handleRadius);
    // left is [-]
    // right is [+]
}

void HorizontalJoystickWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QPointF center(width() / 2.0, height() / 2.0);

    p.setBrush(QColor(30, 30, 30));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(1, 1, width(), height(), 20, 20);


    QPen borderPen(QColor("#FC9997"));
    borderPen.setWidth(4);
    p.setPen(borderPen);
    p.setBrush(QColor(30, 30, 30));
    p.drawRoundedRect(1, 1, width()-1, height()-1, 20, 20);


    QPointF handleCenter = center + handlePos;
    p.setBrush(QColor(255, 251, 227));
    p.drawEllipse(handleCenter, handleRadius, handleRadius);
}

void HorizontalJoystickWidget::mousePressEvent(QMouseEvent *event){
    dragging = true;
    updateHandle(event->pos());
}

void HorizontalJoystickWidget::mouseMoveEvent(QMouseEvent *event){
    if (dragging) {
        updateHandle(event->pos());
    }
}

void HorizontalJoystickWidget::mouseReleaseEvent(QMouseEvent *){
    dragging = false;
    handlePos = QPointF(0, 0);
    update();
    emit joystickMoved(getNormalizedVector());
}

void HorizontalJoystickWidget::updateHandle(const QPointF &pos){
    QPointF center(width() / 2.0, height() / 2.0);
    QPointF delta = pos - center;

    qreal maxDistance = w - handleRadius*2;
    if (QLineF(0, 0, delta.x(), delta.y()).length() > maxDistance) {
        delta = delta / std::hypot(delta.x(), delta.y()) * maxDistance;
    }

    delta.setY(0.0);
    handlePos = delta;
    update_bool.store(true);
    update();
    // emit joystickMoved(getNormalizedVector());
}
