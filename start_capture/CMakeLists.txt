cmake_minimum_required(VERSION 3.0.2)
project(start_capture)

find_package(catkin REQUIRED COMPONENTS 
  message_generation
  std_msgs
)

add_service_files(
  DIRECTORY srv
  FILES capture_images.srv)

generate_messages(
  DEPENDENCIES
  std_msgs
)

catkin_package(CATKIN_DEPENDS std_msgs)