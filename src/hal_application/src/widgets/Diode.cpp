#include "widgets/Diode.h"


Diode::Diode(bool start_status, QWidget *parent): QWidget(parent), status(start_status), danger_status(false) {
    if (status) { current_color = enable_color; } else { current_color = disable_color; }
}

void Diode::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (status) { current_color = enable_color; } else { current_color = disable_color; }
    if (danger_status) { current_color = danger_color; }
    painter.setBrush(current_color);
    painter.setPen(QPen(Qt::black, 2));
    int diameter = qMin(width(), height());
    QRectF rect((width() - diameter) / 2.0, (height() - diameter) / 2.0, diameter, diameter);
    painter.drawEllipse(rect);
}


// SLOTS
void Diode::enable() {
    status = true;
    danger_status = false;
    update();
}
void Diode::disable() {
    status = false;
    danger_status = false;
    update();
}
void Diode::toggle() {
    status = !status;
    update();
}
void Diode::danger() {
    danger_status = true;
    update();
}