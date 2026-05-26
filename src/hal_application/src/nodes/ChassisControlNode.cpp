#include "nodes/ChassisControlNode.h"


ChassisControlNode::ChassisControlNode(QObject *parent) : ApplicationNode(parent) {
    std::cout << "[CHASSIS CONTROL NODE][INITIALIZATION]" << std::endl;
    active.store(true);
}

ChassisControlNode::~ChassisControlNode() {
    std::cout << "[CHASSIS CONTROL NODE][TOTAL DESTRUCTION]" << std::endl;
}


void ChassisControlNode::run() {
    node = std::make_shared<rclcpp::Node>("ChassisControlNode");
    velocity_publisher = node->create_publisher<geometry_msgs::msg::Twist>("/app/cmd_vel", 10);
    gamepad_subscriber = node->create_subscription<hal_interfaces::msg::GamepadInterface>("/gamepad_data",10,std::bind(&ChassisControlNode::gamepadHandler, this, std::placeholders::_1));
    alive_subscriber = node->create_subscription<example_interfaces::msg::Bool>("/comm_status",10,[this] (std::shared_ptr<example_interfaces::msg::Bool> msg) {comm_alive = true;});
    light_publisher = node->create_publisher<example_interfaces::msg::String>("/light",10);
    init_publisher = node->create_publisher<example_interfaces::msg::Bool>("/init_chassis",10);
    
    publish_timer = node->create_wall_timer(
        std::chrono::milliseconds(50),
        [this]() {
            if (active.load()) {
                velocity_publisher->publish(current_velocity);
                emit telemetry(current_velocity.linear.x, current_velocity.angular.z);
            }
        }
    );
    alive_timer = node->create_wall_timer(
        std::chrono::seconds(1),
        [this]() {
            emit aliveStatus(comm_alive);
            comm_alive = false;
        }
    );

    RCLCPP_INFO(node->get_logger(), "[CHASSIS CONTROL NODE][SUCCESSFULL INITIALIZATION]");

    executor.add_node(node);
    executor.spin();
}

void ChassisControlNode::gamepadHandler(std::shared_ptr<hal_interfaces::msg::GamepadInterface> msg) {
    if (gamepad_active.load()) {
        if (msg->axes[5] > -0.5) {
            current_velocity.linear.x = (msg->axes[5] + 0.5) / 1.7; // [0 - 1];
        } else if (msg->axes[2] > -0.5) {
            current_velocity.linear.x = - (msg->axes[2] + 0.5) / 1.7;   // [-1 - 0];
        }
        else {
            current_velocity.linear.x = 0.0;
        }
        if (std::abs(msg->axes[0]) > 0.1) {
            current_velocity.angular.z = - (msg->axes[0]) / 3;
        } else {
            current_velocity.angular.z = 0.0;
        }
    }
}



// SLOTS
void ChassisControlNode::updateVelocity(QPointF velocity_vector) {
    current_velocity.linear.x = -float(velocity_vector.y());
    current_velocity.angular.z = -float(velocity_vector.x()) / 3;
}
void ChassisControlNode::zeroVelocity() {
    current_velocity.linear.x = 0.0;
    current_velocity.angular.z = 0.0;
}
void ChassisControlNode::armChassis() {
    auto msg = example_interfaces::msg::Bool();
    init_publisher->publish(msg);
}
void ChassisControlNode::enableGamepad() {
    gamepad_active.store(true);
    RCLCPP_INFO(node->get_logger(), "[GAMEPAD ENABLED]");
    emit gamepadStatus(gamepad_active.load());
}
void ChassisControlNode::disableGamepad() {
    gamepad_active.store(false);
    RCLCPP_INFO(node->get_logger(), "[GAMEPAD DISABLED]");
    emit gamepadStatus(gamepad_active.load());
}
void ChassisControlNode::lightChange(QString qstr) {
    auto msg = example_interfaces::msg::String();
    std::string s = qstr.toStdString();
    if (s == "red") {
        msg.data = "red";
    } else if (s == "green") {
        msg.data = "green";
    } else if (s == "blue") {
        msg.data = "blue";
    }
    light_publisher->publish(msg);
}

