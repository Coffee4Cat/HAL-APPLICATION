cmake_minimum_required(VERSION 3.8)
project(hal_application)

set(APP_NAME application)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS
  ${CMAKE_CURRENT_SOURCE_DIR}/forms
)
set(CMAKE_AUTORCC ON)

# dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(hal_interfaces REQUIRED)
find_package(example_interfaces REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(action_msgs REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets Gui Core)

file(GLOB_RECURSE APP_SOURCES
  src/*.cpp
  include/*.h
  forms/*.ui
)

add_executable(${APP_NAME} ${APP_SOURCES})

target_include_directories(${APP_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}
)

target_link_libraries(${APP_NAME}
  Qt5::Widgets
  Qt5::Gui
  Qt5::Core
)

ament_target_dependencies(${APP_NAME}
  rclcpp
  std_msgs
  geometry_msgs
  sensor_msgs
  example_interfaces
  hal_interfaces
  action_msgs
  ament_index_cpp
)

install(TARGETS
  ${APP_NAME}
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY forms maps predefined_paths launch
  DESTINATION share/${PROJECT_NAME}/
)

ament_python_install_package(${PROJECT_NAME})
install(PROGRAMS
  scripts/gamepad_interface.py
  DESTINATION lib/${PROJECT_NAME}
)



ament_package()
