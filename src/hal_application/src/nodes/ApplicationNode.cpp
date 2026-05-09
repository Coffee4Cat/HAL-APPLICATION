#include <nodes/ApplicationNode.h>


ApplicationNode::ApplicationNode(QObject *parent) : QThread(parent) {
    active.store(false);
}

ApplicationNode::~ApplicationNode() {
    rclcpp::shutdown();
    wait();
}

void ApplicationNode::activate() {active.store(true);}
void ApplicationNode::deactivate() {active.store(false);}



// SLOTS
void ApplicationNode::set_status(bool new_status) {
    if (new_status) { activate(); } else { deactivate(); }
    emit status(active.load());
}
void ApplicationNode::get_status() {
    emit status(active.load());
}
void ApplicationNode::toggle_status() {
    if (active.load()) { deactivate(); } else { activate(); }
    emit status(active.load());
}





