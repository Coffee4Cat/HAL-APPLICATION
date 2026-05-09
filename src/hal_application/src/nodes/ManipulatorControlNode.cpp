#include "nodes/ManipulatorControlNode.h"
#include "CONFIG.h"

// HELPER FUNCTIONS - BEGIN
std::vector<double> quaternion_mul(const std::vector<double>& q1, const std::vector<double>& q2) {
    return {
        q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3],  // w
        q1[0]*q2[1] + q1[1]*q2[0] + q1[2]*q2[3] - q1[3]*q2[2],  // x
        q1[0]*q2[2] - q1[1]*q2[3] + q1[2]*q2[0] + q1[3]*q2[1],  // y
        q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1] + q1[3]*q2[0]   // z
    };
}
std::vector<double> normalize_vector(const std::vector<double>& v) {
    double norm = std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    return {v[0]/norm, v[1]/norm, v[2]/norm};
}
std::vector<double> normalize_quaternion(const std::vector<double>& q) {
    double norm = std::sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
    return {q[0]/norm, q[1]/norm, q[2]/norm, q[3]/norm};
}
// HELPER FUNCTIONS - END
// HELPER STRUCTS - BEGIN
struct Limit {
    double min;
    double max;
};
// HELPER STRUCTS - END




ManipulatorControlNode::ManipulatorControlNode(QObject *parent) : ApplicationNode(parent) {
    std::cout << "[MANIPULATOR CONTROL NODE][INITIALIZATION]" << std::endl;
    std::vector<double> angles = {0.0, 0.08, 0.15, 1.22, 0.3, -2.5};
    current_joint_state.position = angles;
}

ManipulatorControlNode::~ManipulatorControlNode() {
    std::cout << "[MANIPULATOR CONTROL NODE][TOTAL DESTRUCTION]" << std::endl;
}

void ManipulatorControlNode::run() {
    node = std::make_shared<rclcpp::Node>("ManipulatorControlNode");
    motion_sequencer = std::make_unique<MotionSequencer>(this);
    gamepad_subscriber = node->create_subscription<hal_interfaces::msg::GamepadInterface>("/gamepad_data", 10, std::bind(&ManipulatorControlNode::gamepadHandler, this, std::placeholders::_1));
    init_publisher = node->create_publisher<example_interfaces::msg::Bool>("/init_manipulator",10);
    // motion_sequencer(this)


    joint_state_publisher = node->create_publisher<sensor_msgs::msg::JointState>("/joint_states", 10);
    gripper_publisher = node->create_publisher<example_interfaces::msg::String>("/gripper", 10);
    inverse_kinematics_client = node->create_client<hal_interfaces::srv::InverseKinematics>("/inverse_kinematics");
    forward_kinematics_client = node->create_client<hal_interfaces::srv::ForwardKinematics>("/forward_kinematics");
    publish_timer = node->create_wall_timer(
        std::chrono::milliseconds(150),
        [this]() {
            if (active.load()) {
                joint_state_publisher->publish(current_joint_state);
                emit telemetry(this->translation[0],this->translation[1],this->translation[2],
                    this->orientation[0],this->orientation[1],this->orientation[2],this->orientation[3]);
            }
        }
    );
    RCLCPP_INFO(node->get_logger(), "[MANIPULATOR CONTROL NODE][SUCCESSFULL INITIALIZATION]");
    executor.add_node(node);
    executor.spin();

}

void ManipulatorControlNode::gamepadHandler(std::shared_ptr<hal_interfaces::msg::GamepadInterface> msg) {
    if (gamepad_active.load()) {

        double delta = 0.0;

        static int gripper_state = 0; // -1 = closing / 0 = stop / 1 = opening
        if (static_cast<int>(msg->buttons[4]) == 1 && static_cast<int>(msg->buttons[5]) == 0 && gripper_state != 1) {
            gripperOpen();
            gripper_state = 1;
        } else if (static_cast<int>(msg->buttons[4]) == 0 && static_cast<int>(msg->buttons[5]) == 1 && gripper_state != -1) {
            gripperClose();
            gripper_state = -1;
        } else if (static_cast<int>(msg->buttons[4]) == 0 && static_cast<int>(msg->buttons[5]) == 0 && gripper_state != 0){
            gripperStop();
            gripper_state = 0;
        }

        if (std::abs(msg->axes[0]) > 0.1) {
            delta = - (msg->axes[0] * motion_mul);
            translation[1] += delta; 
        }
        if (std::abs(msg->axes[1]) > 0.1) {
            if (mode45) {
                delta = - (msg->axes[1] * motion_mul) * std::cos(0.785398);
                translation[0] += delta;
                delta = (msg->axes[1] * motion_mul) * std::sin(0.785398);
                translation[2] += delta;
            } else {
                delta = - (msg->axes[1] * motion_mul);
                translation[0] += delta;  
            }
        }
        if (std::abs(msg->axes[4]) > 0.1) {
            if (mode45) {
                delta = - (msg->axes[4] * motion_mul) * std::sin(0.785398);
                translation[0] += delta;
                delta = - (msg->axes[4] * motion_mul) * std::cos(0.785398);
                translation[2] += delta;
            } else {
                delta = - (msg->axes[4] * motion_mul);
                translation[2] += delta;  
            }
        }
        callKinematicsService();

        if (msg->axes[2] > -0.5) {
            delta = - (msg->axes[2] + 0.5) * motion_mul;
            if (active.load()) {
                rotate(2,delta);
                callKinematicsService();
            }
        }
        if (msg->axes[5] > -0.5) {
            delta = (msg->axes[5] + 0.5) * motion_mul;
            if (active.load()) {
                rotate(2,delta);
                callKinematicsService();
            }
        }
        if (static_cast<int>(msg->hat_one) == 1) {
            delta = 0.01;
            if (active.load()) {
                rotate(0, delta);
                callKinematicsService();
            }
        } else if (static_cast<int>(msg->hat_one) == -1) {
            delta = -0.01;
            if (active.load()) {
                rotate(0, delta);
                callKinematicsService();
            }
        }
        if (static_cast<int>(msg->hat_two) == 1) {
            delta = 0.01;
            if (active.load()) {
                rotate(1, delta);
                callKinematicsService();
            }
        } else if (static_cast<int>(msg->hat_two) == -1) {
            delta = -0.01;
            if (active.load()){
                rotate(1, delta);
                callKinematicsService(); 
            }
        }
        // callKinematicsService();
    }

}




void ManipulatorControlNode::rotate(int axis, double angle) {
    std::vector<double> loc_quat;
    switch (axis)
    {
    case 0:
        loc_quat = {0.0,1.0,0.0,0.0};break;
    case 1:
        loc_quat = {0.0,0.0,1.0,0.0};break;
    case 2:
        loc_quat = {0.0,0.0,0.0,1.0};break;
    default:
        break;
    }
    std::vector<double> orientation_conj = {orientation[0], -orientation[1], -orientation[2], -orientation[3]};
    std::vector<double> temp = quaternion_mul(orientation, loc_quat);
    std::vector<double> global_quat = quaternion_mul(temp, orientation_conj);
    std::vector<double> global_vec = {global_quat[1], global_quat[2], global_quat[3]};
    global_vec = normalize_vector(global_vec);
    std::vector<double> rotation = {std::cos(angle/2), global_vec[0]*std::sin(angle/2), global_vec[1]*std::sin(angle/2), global_vec[2]*std::sin(angle/2)};
    orientation = quaternion_mul(rotation, orientation);
    orientation = normalize_quaternion(orientation);
}

void ManipulatorControlNode::checkLimits() {
    // not elegant, but works... TODO REPAIR AFTER ERC2025
    static std::vector<Limit> limits = {
                {-1.65, 1.57},
                {2.56, 0.735},
                {-0.415, 1.606},
                {-4.36, 1.92},
                {-1.32, 1.58},
                {-3.64, 8.78}
            };

    std::vector<int> limit_status = {};
    double min_lim = 0.0;
    double max_lim = 0.0;
    std::vector<double> dofs = static_cast<std::vector<double>>(current_joint_state.position);
    for (size_t i = 0; i < dofs.size(); i++) {
        min_lim = std::abs(limits[i].min - dofs[i]);
        max_lim = std::abs(limits[i].max - dofs[i]);
        if (min_lim < 0.5 || max_lim < 0.5) { limit_status.push_back(2); }      // red (disable diode)
        else if (min_lim < 0.9 || max_lim < 0.9) { limit_status.push_back(1); } // yellow (danger diode)
        else { limit_status.push_back(0); }                                     // green (everything alr) 
    }
    emit dofStatus(limit_status[0], limit_status[1], limit_status[2], limit_status[3], limit_status[4], limit_status[5]);
}



void ManipulatorControlNode::callKinematicsService() {
    geometry_msgs::msg::Pose pose;
    pose.position.x = translation[0];
    pose.position.y = translation[1];
    pose.position.z = translation[2];
    pose.orientation.w = orientation[0];
    pose.orientation.x = orientation[1];
    pose.orientation.y = orientation[2];
    pose.orientation.z = orientation[3];

    auto request = std::make_shared<hal_interfaces::srv::InverseKinematics::Request>();
    request->pose = pose;
    auto future = inverse_kinematics_client->async_send_request(
        request,
        std::bind(&ManipulatorControlNode::futureCallbackKinematics, this, std::placeholders::_1)
    );
}

void ManipulatorControlNode::futureCallbackKinematics(rclcpp::Client<hal_interfaces::srv::InverseKinematics>::SharedFuture future) {
    auto response = future.get();
    if (response->success){
        current_joint_state = response->joints;
        checkLimits();
        safe_orientation = orientation;
        safe_translation = translation;
    } else {
        orientation = safe_orientation;
        translation = safe_translation;
    }
}



// slots
void ManipulatorControlNode::updateXYT(QPointF point) {
    double delta_x;
    double delta_z;
    if (mode45) {
        delta_x = -point.y() * motion_mul * std::cos(0.785398);
        delta_z = point.y() * motion_mul * std::sin(0.785398);
    } else {
        delta_x = -point.y() * motion_mul;
        delta_z = 0.0;
    }
    double delta_y = -point.x() * motion_mul;
    if (active.load()){
        translation[0] += delta_x;
        translation[1] += delta_y;
        translation[2] += delta_z;
        callKinematicsService();
    }
}
void ManipulatorControlNode::updateZT(QPointF point) {
    double delta_x;
    double delta_z;
    if (mode45) {
        delta_x = -point.y() * motion_mul * std::sin(0.785398);
        delta_z = -point.y() * motion_mul * std::cos(0.785398);
    } else {
        delta_x = 0.0;
        delta_z = -point.y() * motion_mul;
    }
    if (active.load()){
        translation[0] += delta_x;
        translation[2] += delta_z;
        callKinematicsService();
    }
}
void ManipulatorControlNode::updateXO(QPointF point) {
    double delta_x = -point.x() * motion_mul;
    if (active.load()) {
        rotate(0,delta_x);
        callKinematicsService();
    }
}
void ManipulatorControlNode::updateYO(QPointF point) {
    double delta_y = -point.y() * motion_mul;
    if (active.load()) {
        rotate(1,delta_y);
        callKinematicsService();
    }
}
void ManipulatorControlNode::updateZO(QPointF point) {
    double delta_z = -point.x() * motion_mul;
    if (active.load()) {
        rotate(2,delta_z);
        callKinematicsService();
    }
}
void ManipulatorControlNode::gripperOpen() {
    auto msg = example_interfaces::msg::String();
    msg.data = "open";
    gripper_publisher->publish(msg);
}
void ManipulatorControlNode::gripperClose() {
    auto msg = example_interfaces::msg::String();
    msg.data = "close";
    gripper_publisher->publish(msg);
}
void ManipulatorControlNode::gripperStop() {
    auto msg = example_interfaces::msg::String();
    msg.data = "stop";
    gripper_publisher->publish(msg);
    gripper_publisher->publish(msg);
}
void ManipulatorControlNode::toggleMode45() {
    mode45 = !mode45;
    emit mode45Status(mode45);
}



void ManipulatorControlNode::initManipulator() {
    auto msg = example_interfaces::msg::Bool();
    init_publisher->publish(msg);
}
void ManipulatorControlNode::enableGamepad() {
    gamepad_active.store(true);
    RCLCPP_INFO(node->get_logger(), "[GAMEPAD ENABLED]");
    emit gamepadStatus(gamepad_active.load());
}
void ManipulatorControlNode::disableGamepad() {
    gamepad_active.store(false);
    RCLCPP_INFO(node->get_logger(), "[GAMEPAD DISABLED]");
    emit gamepadStatus(gamepad_active.load());
}
void ManipulatorControlNode::frontPreset() {
    orientation = {0.707, 0.0, 0.707, 0.0};
    RCLCPP_INFO(node->get_logger(), "[FRONT PRESET]");
    callKinematicsService();
}
void ManipulatorControlNode::groundPreset() {
    orientation = {0.0, 0.0, 1.0, 0.0};   // MEASUREMENTS NEEDED
    RCLCPP_INFO(node->get_logger(), "[GROUND PRESET]");
    callKinematicsService();
}
void ManipulatorControlNode::home() {
    translation = {0.7, 0.0, 1.0};
    orientation = {0.707, 0.0, 0.707, 0.0};
    RCLCPP_INFO(node->get_logger(), "[HOME]");
    callKinematicsService();
}
void ManipulatorControlNode::moveToPreset(std::string preset_name) {
    std::vector<float> preset = presetMap.at(preset_name);
    translation = {preset[0], preset[1], preset[2]};
    orientation = {preset[3], preset[4], preset[5], preset[6]};
    RCLCPP_INFO(node->get_logger(), "[%s]",preset_name.c_str());
    callKinematicsService();
}
void ManipulatorControlNode::executeSequence(std::string sequence_name) {
    const std::vector<std::vector<float>>* sequence = &motionMap.at(sequence_name);
    motion_sequencer->executeSequence(sequence);
}

void ManipulatorControlNode::changeMotionMul(int new_mul) {
    // this is really bad, but it works... TODO REPAIR AFTER ERC2025   (COMMENT GENERATED BY COPILOT xDDDDDDDDD)
    motion_mul = static_cast<float>(new_mul) / 1000.0f;
}



MotionSequencer::MotionSequencer(ManipulatorControlNode* manipulator_control_node): manipulator_control_node(manipulator_control_node), motion_sequence(nullptr) {}
void MotionSequencer::executeSequence(const std::vector<std::vector<float>>* new_motion_sequence) {
    // motion_sequence = new_motion_sequence;
    // tutaj ten thread pod jakąc zmienną aby można było cancelować sekewncje...
    std::thread([this, new_motion_sequence]() {
        for (auto& pose : *new_motion_sequence) {
            manipulator_control_node->translation = {pose[0], pose[1], pose[2]};
            manipulator_control_node->orientation = {pose[3], pose[4], pose[5], pose[6]};
            
            std::cout << "ELO" << std::endl;
            manipulator_control_node->callKinematicsService();
            std::this_thread::sleep_for(std::chrono::milliseconds(int(pose[7] * 1000)));
        }
    }).detach();
}




