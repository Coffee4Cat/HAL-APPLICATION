#pragma once
#include <QObject>
#include <QThread>
#include <QString>
#include <QPointF>
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <memory>
#include <atomic>
#include <string>

class ApplicationNode : public QThread {
    Q_OBJECT

private:
    void activate();
    void deactivate();

public:
    ApplicationNode(QObject *parent = nullptr);
    virtual ~ApplicationNode();
    virtual void run() override = 0;
    
signals:
    void status(bool current_status);
public slots:
    void set_status(bool new_status);
    void toggle_status();
    void get_status();

protected:
    std::atomic<bool> active{false};
    std::shared_ptr<rclcpp::Node> node;
    rclcpp::executors::SingleThreadedExecutor executor;
};