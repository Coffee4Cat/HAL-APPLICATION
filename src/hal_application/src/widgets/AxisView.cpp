#include "widgets/AxisView.h"


AxisView::AxisView(QWidget *parent) : QWidget(parent) {}


void AxisView::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    static QPointF center = QPointF(width() / 2.0, height() / 2.0);
    static double R = width() / 2.0;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(30, 30, 30));
    
    
    painter.setPen(QPen(Qt::green, 3));
    painter.drawLine(QPointF(center.x() - R * std::cos(angle), center.y() + R * std::sin(angle)), QPointF(center.x() + R * std::cos(angle), center.y() - R * std::sin(angle)));
    
    painter.setPen(QPen(Qt::blue, 3));
    painter.drawLine(QPointF(center.x() - R * (std::cos(angle + 1.57)), center.y() + R * (std::sin(angle + 1.57))), QPointF(center.x() + R * (std::cos(angle + 1.57)), center.y() - R * (std::sin(angle + 1.57))));
    
    painter.setBrush(Qt::red);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(center, 5, 5);

}


void AxisView::rotate(double d6) {
    // it needs full quat to perform calculations... hell nah
    angle = d6 - zero_reference;
    update();
}



