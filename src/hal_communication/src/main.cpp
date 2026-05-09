#include <exception>
#include <iostream>
#include <chrono>
#include "rclcpp/rclcpp.hpp"
#include "hal_communication/comm_server.h"

/**
 * @brief Function prints red formatted ROS2-style error message
 * @param msg error message
 */
void printError(const char *msg) {
  std::cerr << "\033[0;31m";
  std::cerr << "[ ERROR] [" << std::chrono::system_clock::now().time_since_epoch().count()
            << "] Communication node failed with message: " << msg;
  std::cerr << "\033[0m\n";
}

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  try {
    auto node = std::make_shared<HAL::CommunicationServer>();
    rclcpp::spin(node);
  } catch (const std::runtime_error &ex) {
    // Fallback error printing if logging is not available
    printError(ex.what());
    return 1;
  }

  rclcpp::shutdown();
  return 0;
}