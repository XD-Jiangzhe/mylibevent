cmake_minimum_required(VERSION 3.5)
project(main)



set(SRC_LIST main.cc epoller.cc channel.cc eventloop.cc listener.cc thread_pool.cc tcpserver.cc )

set(CMAKE_CXX_FLAGS "-g -std=c++11 -fno-common")

# add_definitions(-DDEBUG)

add_executable(main ${SRC_LIST})
target_link_libraries(main pthread)