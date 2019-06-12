cmake_minimum_required(VERSION 2.8.3)
project(vcs_con)

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
 )
## Generate added messages and services with any dependencies listed here
 generate_messages(
   DEPENDENCIES
   std_msgs
 )

catkin_package(
#  INCLUDE_DIRS include
  LIBRARIES vcs_con
  CATKIN_DEPENDS roscpp std_msgs
  DEPENDS system_lib
)

include_directories(
 include
  ${catkin_INCLUDE_DIRS}
)
add_executable(${PROJECT_NAME} src/main.cpp)
add_dependencies(${PROJECT_NAME} vcs_con_generate_messages_cpp)
target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES})
