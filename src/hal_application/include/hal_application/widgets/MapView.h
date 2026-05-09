#pragma once
#include <iostream>
#include <QGraphicsPolygonItem>
#include <QGraphicsPathItem>
#include <QPolygonF>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsLineItem>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QPen>
#include <QPixmap>
#include <QPointF>
#include <QLabel>
#include <QString>
#include <array>
#include <map>
#include <vector>
#include <QFont>
#include "ament_index_cpp/get_package_share_directory.hpp"
#include "CONFIG.h"


class ArrowItem : public QGraphicsPolygonItem {
public:
    explicit ArrowItem(double size = pixels_per_meter, QColor color = QColor(0,0,255));
    void setArrowPos(double x, double y, double angle, double pixels_per_meter);
    double real_x;
    double real_y;
    double display_x;
    double display_y;
};
class StarItem : public QGraphicsPolygonItem {
public:
    explicit StarItem(double size = pixels_per_meter, QColor color = QColor(0,0,255));
    void setStarPos(double x, double y, double pixels_per_meter);
    double real_x;
    double real_y;
    double display_x;
    double display_y;
};
class PathItem : public QGraphicsPathItem {
public:
    QPainterPath path;
    QPointF old_point;
    QPointF new_point;
    explicit PathItem();
    void addPoint(double x, double y, double pixels_per_meter);
    void clearPath();
};
class BlockItem : public QGraphicsPolygonItem {
public:
    explicit BlockItem(int index, double size = pixels_per_meter * 0.7, QColor color = QColor(250, 148, 0));
    void setBlockPos(double x, double y, double pixels_per_meter);
    double real_x;
    double real_y;
    double display_x;
    double display_y;
};






class MapView : public QGraphicsView {
    Q_OBJECT

public:
    explicit MapView(QWidget* parent = nullptr);
    void loadMap(const QString& path);
    QGraphicsScene* scene;
    QGraphicsPixmapItem* map_item;
    QPoint last_mouse_pos;
    double scale_factor = 6.0;
    // double scale_factor = 60.0;

    // COSTMAP STUFF - BEGIN
    // double pixels_per_meter = 5.0; // ??????????
    // double zero[2] = {1.0,10.5};
    // double zero[2] = {6.0,19.0};
    // COSTMAP STUFF - END

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    double x_cord = 0.0;
    double y_cord = 0.0;
    PathItem* rover_path;
    ArrowItem* rover;
    StarItem* current_goal;
    StarItem* point_zero;
    QLabel* cursor_cord;
    std::vector<BlockItem*> aruco_blocks;

    void drawGrid();

public slots:
    void updateRoverPos(double x, double y, double angle);
    void updateGoalPos(double x, double y);
    void clearPath();
};