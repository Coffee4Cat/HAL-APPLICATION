#pragma once

#include <QWidget>
#include <QPainter>

class Diode : public QWidget {
    Q_OBJECT

private:
    bool status;
    bool danger_status;
    QColor disable_color = QColor(247, 172, 172);
    QColor enable_color = QColor(114, 255, 197);
    QColor danger_color = QColor(245, 243, 126); // DOF WARNING FEATURE
    QColor current_color;

public:
    explicit Diode(bool start_status = false, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

public slots:
    void enable();
    void danger();
    void disable();
    void toggle();
};