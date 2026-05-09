#pragma once
#include <QWidget>
#include <cmath>
#include <QPainter>

class AxisView : public QWidget {
    Q_OBJECT

private:
    double zero_reference = -2.5016060419903905;
    double angle = 0.0;

public:
    explicit AxisView(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

public slots:
    void rotate(double d6);
};