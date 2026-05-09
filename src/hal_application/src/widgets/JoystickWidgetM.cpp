#include "widgets/JoystickWidgetM.h"


JoystickWidgetM::JoystickWidgetM(QWidget *parent): QWidget(parent), dragging(false), handlePos(0, 0) {
    setMinimumSize(2 * radius, 2 * radius);
    updateTimer.setInterval(100);
    connect(&updateTimer, &QTimer::timeout, this, [this]() {
        constexpr qreal EPSILON = 1e-5;
        if (update_bool.load()) {
            emit joystickMoved(getNormalizedVector());
            if (std::abs(handlePos.y()) < EPSILON && std::abs(handlePos.x()) < EPSILON) {
                update_bool.store(false);
            }
        }
    });
    updateTimer.start();
}

QPointF JoystickWidgetM::getNormalizedVector() const {
    return handlePos / (radius - handleRadius);
}

void JoystickWidgetM::paintEvent(QPaintEvent *) {
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

void JoystickWidgetM::mousePressEvent(QMouseEvent *event){
    dragging = true;
    updateHandle(event->pos());
}

void JoystickWidgetM::mouseMoveEvent(QMouseEvent *event){
    if (dragging) {
        updateHandle(event->pos());
    }
}

void JoystickWidgetM::mouseReleaseEvent(QMouseEvent *){
    dragging = false;
    handlePos = QPointF(0, 0);
    update();
    emit joystickMoved(getNormalizedVector());
}

void JoystickWidgetM::updateHandle(const QPointF &pos){
    QPointF center(width() / 2.0, height() / 2.0);
    QPointF delta = pos - center;

    qreal maxDistance = radius - handleRadius;
    if (QLineF(0, 0, delta.x(), delta.y()).length() > maxDistance) {
        delta = delta / std::hypot(delta.x(), delta.y()) * maxDistance;
    }

    update_bool.store(true);
    handlePos = delta;
    update();
    // emit joystickMoved(getNormalizedVector());
}
