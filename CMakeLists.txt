cmake_minimum_required(VERSION 3.18)

project(GitLab VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS HttpServer)

add_library(gitlab gitlabservice.h gitlabservice.cpp)

target_link_libraries(gitlab PUBLIC Qt::Core)
target_link_libraries(gitlab PRIVATE Qt::HttpServer)
target_include_directories(gitlab PUBLIC ${PROJECT_SOURCE_DIR})
