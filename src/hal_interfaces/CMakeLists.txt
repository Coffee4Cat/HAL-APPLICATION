cmake_minimum_required(VERSION 3.8)
project(hal_interfaces)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

# Specify service files to be built
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/ArucoPose.msg"
  "msg/GamepadInterface.msg"
  "srv/Communication.srv"
  "srv/ArucoPose.srv"
  "msg/Transmission.msg"
  "srv/InverseKinematics.srv"
  "srv/ForwardKinematics.srv"
  "msg/GroundObjectPose.msg"
  "srv/PoseGNSS.srv"
  DEPENDENCIES std_msgs geometry_msgs sensor_msgs
)

# ament_export_dependencies(rosidl_default_runtime)

# Install the package.xml file
# install(DIRECTORY srv
#   DESTINATION share/${PROJECT_NAME}
# )

ament_package()
