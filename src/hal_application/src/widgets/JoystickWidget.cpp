#include "widgets/JoystickWidget.h"


JoystickWidget::JoystickWidget(QWidget *parent): QWidget(parent), dragging(false), handlePos(0, 0) {
    setMinimumSize(2 * radius, 2 * radius);
}

QPointF JoystickWidget::getNormalizedVector() const {
    return handlePos / (radius - handleRadius);
}

void JoystickWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QPointF center(width() / 2.0, height() / 2.0);

    p.setBrush(QColor(30, 30, 30));
    p.setPen(Qt::NoPen);
    p.drawEllipse(center, radius, radius);

    QPen borderPen(QColor("#FC9997"));
    borderPen.setWidth(4);
    p.setPen(borderPen);
    p.setBrush(QColor(30, 30, 30));
    p.drawEllipse(center, radius, radius);

    QPointF handleCenter = center + handlePos;
    p.setBrush(QColor(255, 251, 227));
    p.drawEllipse(handleCenter, handleRadius, handleRadius);
}

void JoystickWidget::mousePressEvent(QMouseEvent *event){
    dragging = true;
    updateHandle(event->pos());
}

void JoystickWidget::mouseMoveEvent(QMouseEvent *event){
    if (dragging) {
        updateHandle(event->pos());
    }
}

void JoystickWidget::mouseReleaseEvent(QMouseEvent *){
    dragging = false;
    handlePos = QPointF(0, 0);
    update();
    emit joystickMoved(getNormalizedVector());
}

void JoystickWidget::updateHandle(const QPointF &pos){
    QPointF center(width() / 2.0, height() / 2.0);
    QPointF delta = pos - center;

    qreal maxDistance = radius - handleRadius;
    if (QLineF(0, 0, delta.x(), delta.y()).length() > maxDistance) {
        delta = delta / std::hypot(delta.x(), delta.y()) * maxDistance;
    }

    handlePos = delta;
    update();
    emit joystickMoved(getNormalizedVector());
}
