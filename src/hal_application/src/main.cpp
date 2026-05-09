#include <QApplication>
#include "widgets/MainWindow.h"
#include "nodes/ChassisControlNode.h"
#include "nodes/ManipulatorControlNode.h"
#include "nodes/AutonomousNode.h"
#include <thread>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    rclcpp::init(argc,argv);
    ChassisControlNode* chassis_node = new ChassisControlNode();
    ManipulatorControlNode* manipulator_node = new ManipulatorControlNode();
    AutonomousNode* autonomous_node = new AutonomousNode();
    chassis_node->start();
    manipulator_node->start();
    autonomous_node->start();

    QObject::connect(&app, &QApplication::aboutToQuit, [&]() {
        rclcpp::shutdown();
        chassis_node->wait();
        manipulator_node->wait();
        autonomous_node->wait();
        delete chassis_node;
        delete manipulator_node;
        delete autonomous_node;
    });

    MainWindow w(chassis_node, manipulator_node, autonomous_node);
    w.show();

    return app.exec();
}
