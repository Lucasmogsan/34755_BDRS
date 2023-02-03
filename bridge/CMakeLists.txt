cmake_minimum_required(VERSION 3.1)
project(bridge)
#
find_package(OpenCV REQUIRED )
find_package(Threads REQUIRED)

# ROS stuff
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
  message_generation
  tf2_msgs
  tf2_ros
  geometry_msgs
)
#
add_message_files(
   FILES
   battery.msg
)
generate_messages(
    DEPENDENCIES
    std_msgs
)
catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES beginner_tutorials
#  CATKIN_DEPENDS roscpp rospy std_msgs
CATKIN_DEPENDS roscpp rospy std_msgs 
#  message_runtime
#  DEPENDS system_lib
)
## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
# include
  ${catkin_INCLUDE_DIRS}
)
#
# most ROS stuff ends here

include_directories(${OpenCV_INCLUDE_DIRS})
execute_process(COMMAND uname -m RESULT_VARIABLE IS_OK OUTPUT_VARIABLE CPU1)
string(STRIP ${CPU1} CPU)
# works for Raspberry 3 only ??
if (${CPU} MATCHES "armv7l")
   message("got RASPBERRY ${CPU}")
   #    set(EXTRA_CC_FLAGS " -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -DRASPBERRY_PI -D${CPU}")
   set(EXTRA_CC_FLAGS "-D${CPU} -O2 -g0 -DRASPBERRY_PI")
   #set(EXTRA_CC_FLAGS "-D${CPU} -O0 -g2 -DRASPBERRY_PI")
else()
   message("not RASPBERRY ${CPU}")
   set(EXTRA_CC_FLAGS "-D${CPU} -O0 -g2")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic \
    -Wno-format-truncation -Wno-return-type \
    -std=c++17 ${EXTRA_CC_FLAGS}")
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")
#
add_executable(bridge main.cpp utime.cpp ujoy.cpp userverport.cpp userverclient.cpp uconsole.cpp 
      urun.cpp uteensy.cpp udata.cpp uhandler.cpp udataitem.cpp 
      ulogfile.cpp tcpCase.cpp ubridge.cpp usource.cpp usubscribe.cpp 
      uini.cpp uros.cpp uhost.cpp)
# 
target_link_libraries(bridge ${CMAKE_THREAD_LIBS_INIT} readline ${catkin_LIBRARIES})
install(TARGETS bridge RUNTIME DESTINATION bin)

# ROS raw messagse listener
# add_executable(listener ulistener.cpp)
# target_link_libraries(listener ${catkin_LIBRARIES})
# add_dependencies(listener beginner_tutorials_generate_messages_cpp)
