#include "widgets/MapView.h"
#include <QDebug>

constexpr double PI = 3.1415926535897932;

// HELPER FUNCTIONS - BEGIN
std::array<double,2> convertFromROSToDisplay(double x_ros, double y_ros, MapView* map_view) { //  you get coords to put into Qt'setPose' like stuff (in reference to map) 
    std::array<double, 2> disp_cord = {0.0,0.0};
    disp_cord[0] = -y_ros + zero[1];
    disp_cord[1] = -x_ros + double(map_view->scene->sceneRect().height()) / pixels_per_meter - zero[0]; 
    return disp_cord;
}
std::array<double,2> convertFromROSToERC(double x_ros, double y_ros) { // better with it than to forget some minus
    std::array<double, 2> disp_cord = {0.0,0.0};
    disp_cord[0] = x_ros;
    disp_cord[1] = -y_ros; 
    return disp_cord;
}
std::array<double,2> convertFromERCToROS(double x_erc, double y_erc) { // better with it than to forget some minus
    std::array<double, 2> disp_cord = {0.0,0.0};
    disp_cord[0] = x_erc;
    disp_cord[1] = -y_erc; 
    return disp_cord;
}



// HELPER FUNCTIONS - END




ArrowItem::ArrowItem(double size, QColor color) {
    QPolygonF triangle;
    triangle << QPointF(0, -size / 2) << QPointF(size / 4, size / 2) << QPointF(-size / 4, size / 2);
    setPolygon(triangle);
    setBrush(color);
    setPen(QPen(color, 1));
}
void ArrowItem::setArrowPos(double x, double y, double angle, double pixels_per_meter) {
    setPos(x * pixels_per_meter, y * pixels_per_meter);
    setRotation(angle * 180.0 / PI);
}

StarItem::StarItem(double size, QColor color) {
    QPolygonF star;
    star << QPointF(0, -size / 2) << QPointF(size / 2, 0) << QPointF(0, size / 2) << QPointF(-size / 2, 0);;
    setPolygon(star);
    setBrush(color);
    setPen(QPen(color, 1));
}
void StarItem::setStarPos(double x, double y, double pixels_per_meter) {
    setPos(x * pixels_per_meter, y * pixels_per_meter);
}



PathItem::PathItem() {
    QPen pen(Qt::red);
    pen.setWidth(0.9);
    setPen(pen);
    setBrush(Qt::red);
    setZValue(10);

    path = QPainterPath();
    setPath(path);
}
void PathItem::addPoint(double x, double y, double pixels_per_meter) { 
    new_point.setX(x * pixels_per_meter); new_point.setY(y * pixels_per_meter);

    if (QLineF(new_point, old_point).length() > pixels_per_meter/2) {
        path.moveTo(old_point);
        path.lineTo(new_point);
        setPath(path);
        
        old_point.setX(x * pixels_per_meter); old_point.setY(y * pixels_per_meter);
    }
}
void PathItem::clearPath() {
    path = QPainterPath();
    setPath(path);
}


BlockItem::BlockItem(int index, double size, QColor color) {
    QPolygonF block;
    block << QPointF(-size / 2, -size / 2) << QPointF(size / 2, -size / 2) << QPointF(size / 2,size / 2) << QPointF(-size / 2, size / 2);
    setPolygon(block);
    setBrush(color);
    setPen(QPen(color, 1));
    auto textItem = new QGraphicsTextItem(QString::number(index), this);
    textItem->setDefaultTextColor(Qt::black);
    textItem->setFont(QFont("Arial", 2, QFont::Bold));
    QRectF rect = boundingRect();
    textItem->setPos(rect.center() - QPointF(textItem->boundingRect().width() / 2, textItem->boundingRect().height() / 2));
}
void BlockItem::setBlockPos(double x, double y, double pixels_per_meter) {
    setPos(x * pixels_per_meter, y * pixels_per_meter);
}




MapView::MapView(QWidget* parent) : QGraphicsView(parent), map_item(nullptr) {
    scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(Qt::black);
    setScene(scene);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::NoDrag);
    scale(6.0,6.0);
    cursor_cord = new QLabel(this);
    cursor_cord->setStyleSheet("QLabel{color: red; font-size: 15pt;}");
    cursor_cord->setText("COORDINATES:");
    cursor_cord->hide();
    setMouseTracking(true);
    rover = new ArrowItem();
    current_goal = new StarItem(3.0, QColor(65, 232, 140));
    point_zero = new StarItem(2.0, QColor(220, 65, 232));
    rover_path = new PathItem();


}

void MapView::loadMap(const QString& path) {
    QPixmap pixmap(path);
    if (pixmap.isNull()) {
        std::cout << "[MAPVIEW] Path could not be loaded";
    }
    scene->clear();
    map_item = scene->addPixmap(pixmap);
    scene->setSceneRect(pixmap.rect());
    drawGrid();
    rover->setParentItem(map_item);
    current_goal->setParentItem(map_item);
    rover_path->setParentItem(map_item);
    point_zero->setParentItem(map_item);
    std::array<double,2> arr = convertFromROSToDisplay(0.0,0.0,this);
    point_zero->setStarPos(arr[0],arr[1],pixels_per_meter);

    for (auto it = arucoMap.begin(); it != arucoMap.end(); it++) {
        std::array<double,2> arr = convertFromROSToDisplay(it->second[0],-it->second[1],this);
        BlockItem* block_item = new BlockItem(it->first);
        block_item->setParentItem(map_item);
        block_item->setBlockPos(arr[0],arr[1],pixels_per_meter);
        aruco_blocks.push_back(block_item);
    }
}

void MapView::wheelEvent(QWheelEvent* event) {
    constexpr double zoom_in_factor = 1.25;
    constexpr double zoom_out_factor = 0.8;

    if (event->angleDelta().y() > 0 and scale_factor < 25.0) {
        scale(zoom_in_factor, zoom_in_factor);
        scale_factor = scale_factor * zoom_in_factor;
    } else if (event->angleDelta().y() < 0 and scale_factor > 4.9) {
        scale(zoom_out_factor,zoom_out_factor);
        scale_factor = scale_factor * zoom_out_factor;
    }
}

void MapView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        last_mouse_pos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    QGraphicsView::mousePressEvent(event);
}

void MapView::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        QPoint delta = event->pos() - last_mouse_pos;
        last_mouse_pos = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
    }
    QPoint local_cord = event->pos();
    QPointF global_cord = mapToScene(local_cord);

    x_cord = (double(scene->sceneRect().height())-double(global_cord.y()))/pixels_per_meter - zero[0];
    y_cord = double(global_cord.x())/pixels_per_meter - zero[1];

    cursor_cord->setText(QString("COORDINATES:\nX: %1\nY: %2").arg(x_cord).arg(y_cord));
    cursor_cord->move(local_cord - QPoint(175,175));
    QGraphicsView::mouseMoveEvent(event);
}

void MapView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() & Qt::LeftButton) {
        setCursor(Qt::ArrowCursor);
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void MapView::enterEvent(QEvent* event) {
    cursor_cord->show();
    QGraphicsView::enterEvent(event);
}
void MapView::leaveEvent(QEvent* event) {
    cursor_cord->hide();
    QGraphicsView::leaveEvent(event);
}


void MapView::drawGrid() {
    if (!map_item) {return;}

    QPixmap pixmap = map_item->pixmap();
    int width = pixmap.width();
    int height = pixmap.height();
    
    QPen pen(Qt::lightGray);
    pen.setWidth(0);

    for (int x = 0; x <= width; x += pixels_per_meter) {
        scene->addLine(x,0,x,height,pen);
    }
    for (int y = height; y > 0; y -= pixels_per_meter) {
        scene->addLine(0,y,width,y,pen);
    }
}



// SLOTS
void MapView::updateRoverPos(double x, double y, double angle) {
    static bool set_initial_path_point = true;
    std::array<double,2> real = convertFromROSToERC(x,y);
    rover->real_x = real[0];
    rover->real_y = real[1]; // koordnaty te ercowe.
    std::array<double,2> arr = convertFromROSToDisplay(x,y,this);
    rover->display_x = arr[0];
    rover->display_y = arr[1];
    rover->setArrowPos(rover->display_x, rover->display_y, -angle, pixels_per_meter);
    if (set_initial_path_point) {
        rover_path->old_point.setX(rover->display_x * pixels_per_meter); rover_path->old_point.setY(rover->display_y * pixels_per_meter);
        set_initial_path_point = false;
    }
    rover_path->addPoint(rover->display_x, rover->display_y, pixels_per_meter);
    scene->update();

}
void MapView::updateGoalPos(double x, double y) {
    std::array<double,2> arr = convertFromROSToDisplay(x,y,this);
    current_goal->setStarPos(arr[0], arr[1], pixels_per_meter);
}
void MapView::clearPath() {
    rover_path->clearPath();
}


