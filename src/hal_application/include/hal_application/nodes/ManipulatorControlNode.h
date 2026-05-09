#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <cmath>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <example_interfaces/msg/string.hpp>
#include "hal_interfaces/srv/forward_kinematics.hpp"
#include "hal_interfaces/srv/inverse_kinematics.hpp"
#include "hal_interfaces/msg/gamepad_interface.hpp"
#include "nodes/ApplicationNode.h"
#include "example_interfaces/msg/bool.hpp"



class MotionSequencer;


class ManipulatorControlNode : public ApplicationNode {
Q_OBJECT
private:
    // gamepad
    std::shared_ptr<rclcpp::Subscription<hal_interfaces::msg::GamepadInterface>> gamepad_subscriber;
    // std::shared_ptr<rclcpp::Subscription<std_msgs::msg::Bool>> gamepad_status_subscriber;    MAYBE LATER
    std::atomic<bool> gamepad_active{false};
    void gamepadHandler(std::shared_ptr<hal_interfaces::msg::GamepadInterface> msg);

    // joints
    sensor_msgs::msg::JointState current_joint_state;
    std::shared_ptr<rclcpp::Publisher<sensor_msgs::msg::JointState>> joint_state_publisher;
    std::shared_ptr<rclcpp::Publisher<example_interfaces::msg::String>> gripper_publisher;
    std::shared_ptr<rclcpp::Publisher<example_interfaces::msg::Bool>> init_publisher;
    rclcpp::TimerBase::SharedPtr publish_timer;

    // hal_kinematics
    bool mode45 = false;
    std::shared_ptr<rclcpp::Client<hal_interfaces::srv::InverseKinematics>> inverse_kinematics_client;
    std::shared_ptr<rclcpp::Client<hal_interfaces::srv::ForwardKinematics>> forward_kinematics_client;
    std::vector<double> safe_translation = {0.7, 0.0, 1.0}; // X Y Z
    std::vector<double> safe_orientation = {0.707, 0.0, 0.707, 0.0}; // W X Y Z
    void futureCallbackKinematics(rclcpp::Client<hal_interfaces::srv::InverseKinematics>::SharedFuture future);

    // calculations
    std::vector<std::vector<double>> local_axis = {{1.0,0.0,0.0},{0.0,1.0,0.0},{0.0,0.0,1.0}};
    void rotate(int axis, double angle);
    void checkLimits();
    float motion_mul = 0.01;

    //motion sequencer
    std::unique_ptr<MotionSequencer> motion_sequencer;



public slots:
    void updateXYT(QPointF point);
    void updateZT(QPointF point);
    void updateXO(QPointF point);
    void updateYO(QPointF point);
    void updateZO(QPointF point);
    void gripperOpen();
    void gripperClose();
    void gripperStop();
    void initManipulator();
    // gamepad slots
    void enableGamepad();
    void disableGamepad();
    // orientation preset slots
    void frontPreset();
    void groundPreset();
    // mode45 slot
    void toggleMode45();
    // positions presets slots
    void home();
    void moveToPreset(std::string preset_name);
    void executeSequence(std::string sequence_name);
    void changeMotionMul(int new_mul);

signals:
    void gamepadStatus(bool current_status);
    void telemetry(double t0,double t1,double t2,double o0,double o1,double o2,double o3);
    void dofStatus(int d1, int d2, int d3, int d4, int d5, int d6);
    void mode45Status(bool status);
    void dof6Angle(double d6);

public:
    ManipulatorControlNode(QObject *parent = nullptr);
    ~ManipulatorControlNode() override;
    void callKinematicsService();
    std::vector<double> translation = {0.57, -0.14, 0.96}; // X Y Z
    std::vector<double> orientation = {0.707, 0.0, 0.707, 0.0}; // W X Y Z
    void run() override final;

};



class MotionSequencer : public QObject {
private:
    ManipulatorControlNode* manipulator_control_node;
    std::vector<std::vector<float>>* motion_sequence;

public:
    MotionSequencer(ManipulatorControlNode* manipulator_control_node);
    ~MotionSequencer() = default;
    void executeSequence(const std::vector<std::vector<float>>* new_motion_sequence);
};
