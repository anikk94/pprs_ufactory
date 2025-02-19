cmake_minimum_required(VERSION 3.8)
project(pprs_ufactory_moveit)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(rclcpp REQUIRED)

add_executable(moveit_robot_program src/moveit_robot_program.cpp)
target_include_directories(moveit_robot_program PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)
target_compile_features(moveit_robot_program PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
ament_target_dependencies(
  moveit_robot_program
  "moveit_ros_planning_interface"
  "rclcpp"
)

add_executable(
  my_moveit_utils
  src/my_moveit_utils.cpp
)
target_compile_features(
  my_moveit_utils
  PUBLIC
  c_std_99
  cxx_std_17
)
ament_target_dependencies(
  my_moveit_utils
  "moveit_ros_planning_interface"
  "rclcpp"
)

install(
  TARGETS
  moveit_robot_program
  my_moveit_utils
  DESTINATION
  lib/${PROJECT_NAME}
)

install(
  DIRECTORY
  launch
  DESTINATION
  share/${PROJECT_NAME}/
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
