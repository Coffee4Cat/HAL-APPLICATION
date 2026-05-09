
# W/PYTHON

# cmake_minimum_required(VERSION 3.8)
# project(hal_communication)

# if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#   add_compile_options(-Wall -Wextra -Wpedantic)
# endif()

# # find dependencies
# find_package(ament_cmake REQUIRED)
# find_package(ament_cmake_python REQUIRED)
# find_package(rclcpp REQUIRED)
# find_package(rclpy REQUIRED)
# find_package(hal_interfaces REQUIRED)
# # uncomment the following section in order to fill in
# # further dependencies manually.
# # find_package(<dependency> REQUIRED)

# # Include Cpp "include" directory
# include_directories(include)
# include_directories(${hal_interfaces_INCLUDE_DIRS})
# # Create Cpp executable
# add_executable(comm_server src/comm_server.cpp)
# ament_target_dependencies(comm_server rclcpp hal_interfaces)
# add_executable(main src/main.cpp)
# ament_target_dependencies(main rclcpp hal_interfaces)
# # Install Cpp executables
# install(TARGETS
#   main
#   comm_server
#   DESTINATION lib/${PROJECT_NAME}
# )

# # Install Python modules
# ament_python_install_package(${PROJECT_NAME})
# # Install Python executables
# install(PROGRAMS
#   scripts/wheel_interface.py
#   DESTINATION lib/${PROJECT_NAME}
# )

# # Install dirs.
# install(DIRECTORY
#   launch
#   config
#   DESTINATION share/${PROJECT_NAME}/
# )

# ament_package()


# /WO PYTHON


# cmake_minimum_required(VERSION 3.8)
# project(hal_communication)

# # Use C++17 standard
# add_compile_options(-std=c++17)

# # Find required ROS 2 dependencies
# find_package(ament_cmake REQUIRED)
# find_package(rclcpp REQUIRED)
# find_package(std_msgs REQUIRED)
# find_package(hal_interfaces REQUIRED)  # Assuming you have hal_interfaces package

# # Include directories for header files
# include_directories(
#   include
# )

# # Declare a C++ executable
# add_executable(${PROJECT_NAME}_node
#   src/main.cpp
#   src/comm_server.cpp
# )

# # Link the executable with the required libraries
# ament_target_dependencies(${PROJECT_NAME}_node
#   rclcpp
#   std_msgs
#   hal_interfaces  # Add the necessary ROS 2 dependencies
# )

# # Install the executables
# install(TARGETS
#   ${PROJECT_NAME}_node
#   DESTINATION lib/${PROJECT_NAME}
# )

# # Register the package with ament
# ament_package()

cmake_minimum_required(VERSION 3.8)
project(hal_communication)

# Use C++17 standard
add_compile_options(-std=c++17)

# Find required ROS 2 dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(example_interfaces REQUIRED)
find_package(hal_interfaces REQUIRED)  # Assuming you have hal_interfaces package

# Include directories for header files
include_directories(
  include
)

# Declare a C++ executable
add_executable(${PROJECT_NAME}_node
  src/main.cpp
  src/comm_server.cpp
)

# Link the executable with the required libraries
ament_target_dependencies(${PROJECT_NAME}_node
  rclcpp
  std_msgs
  sensor_msgs
  hal_interfaces  # Add the necessary ROS 2 dependencies
)

add_executable(wheel_mux_node
  src/wheel_mux.cpp
)
ament_target_dependencies(wheel_mux_node
  rclcpp
  std_msgs
  sensor_msgs
  rcl_interfaces
)

# Install the executables
install(TARGETS
  ${PROJECT_NAME}_node wheel_mux_node
  DESTINATION lib/${PROJECT_NAME}
)


# Install Python modules

ament_python_install_package(${PROJECT_NAME})

# Install Python executables

install(PROGRAMS
  scripts/wheel_interface.py
  scripts/manipulator_interface.py
  scripts/teleoperator.py
  DESTINATION lib/${PROJECT_NAME}
)


# Install dirs.

install(DIRECTORY
  launch
  config
  DESTINATION share/${PROJECT_NAME}/
)



# Register the package with ament
ament_package()