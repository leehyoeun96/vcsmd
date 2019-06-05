cmake_minimum_required(VERSION 2.8.3)
project(mission_manager)

## Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  message_generation
)
## Generate messages in the 'msg' folder
 add_message_files(
   FILES
   Message.msg
   vcs_msg.msg
 )
## Generate added messages and services with any dependencies listed here
 generate_messages(
   DEPENDENCIES
   std_msgs
 )
catkin_package(
  INCLUDE_DIRS include
  LIBRARIES mission_manager
  CATKIN_DEPENDS roscpp std_msgs
  DEPENDS system_lib
)
include_directories(
 include
  ${catkin_INCLUDE_DIRS}
)
 add_executable(${PROJECT_NAME} src/mission_manager_core.cpp src/mission_manager_node.cpp)
 add_dependencies(${PROJECT_NAME} mision_manager_generate_messages_cpp)

## Specify libraries to link a library or executable target against
 target_link_libraries(${PROJECT_NAME}
   ${catkin_LIBRARIES}
 )

 add_executable(mission_manager_test src/mission_manager_test.cpp src/mission_manager_test_node.cpp)
 add_dependencies(mission_manager_test mision_manager_generate_messages_cpp)

## Specify libraries to link a library or executable target against
 target_link_libraries(mission_manager_test
   ${catkin_LIBRARIES}
 )
