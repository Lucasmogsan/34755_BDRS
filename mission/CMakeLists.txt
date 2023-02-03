cmake_minimum_required(VERSION 3.0)

project(mission)

find_package(OpenCV REQUIRED )
find_package(Threads REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -std=c++17 ${EXTRA_CC_FLAGS}")
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")

add_executable(mission main.cpp 
                            src/ubridge.cpp 
                            src/upose.cpp 
                            src/ucomment.cpp 
                            src/ustate.cpp
                            src/uvision.cpp
                            src/utime.cpp
                            src/uplay.cpp
                            src/uevent.cpp
                            src/ujoy.cpp
                            )

target_link_libraries(mission ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS mission RUNTIME DESTINATION bin)
