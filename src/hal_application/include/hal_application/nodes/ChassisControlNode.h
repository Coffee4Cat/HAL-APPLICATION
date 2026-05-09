#pragma once
#include "nodes/ApplicationNode.h"
#include <geometry_msgs/msg/twist.hpp>
#include <hal_interfaces/msg/gamepad_interface.hpp>
#include <example_interfaces/msg/bool.hpp>
#include <example_interfaces/msg/string.hpp>



class ChassisControlNode : public ApplicationNode {
    Q_OBJECT

private:
    // gamepad
    std::shared_ptr<rclcpp::Subscription<hal_interfaces::msg::GamepadInterface>> gamepad_subscriber;
    // std::shared_ptr<rclcpp::Subscription<std_msgs::msg::Bool>> gamepad_status_subscriber;    MAYBE LATER
    std::atomic<bool> gamepad_active{false};
    void gamepadHandler(std::shared_ptr<hal_interfaces::msg::GamepadInterface> msg);

    // cmd_vel
    std::shared_ptr<rclcpp::Publisher<geometry_msgs::msg::Twist>> velocity_publisher;
    std::shared_ptr<rclcpp::Publisher<example_interfaces::msg::String>> light_publisher;
    std::shared_ptr<rclcpp::Publisher<example_interfaces::msg::Bool>> init_publisher;
    geometry_msgs::msg::Twist current_velocity;
    rclcpp::TimerBase::SharedPtr publish_timer;

    // alive
    std::shared_ptr<rclcpp::Subscription<example_interfaces::msg::Bool>> alive_subscriber;
    rclcpp::TimerBase::SharedPtr alive_timer;
    bool comm_alive;

public:
    ChassisControlNode(QObject *parent = nullptr);
    ~ChassisControlNode() override;
    void run() override final;


public slots:
    void updateVelocity(QPointF velocity_vector);
    void zeroVelocity();
    // gamepad slots
    void enableGamepad();
    void disableGamepad();
    // comm slots
    void armChassis();
    void lightChange(QString qstr);

signals:
    void gamepadStatus(bool current_status);
    void aliveStatus(bool current_status);
    void telemetry(double x, double z);
};