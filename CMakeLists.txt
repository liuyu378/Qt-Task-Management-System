cmake_minimum_required(VERSION 3.16)

project(final_work VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#让CMake能找到ui目录里的.ui 文件
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

#查找Qt
find_package(Qt6 COMPONENTS Widgets Multimedia QUIET)

if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    set(QT_LIB Qt6::Widgets Qt6::Multimedia)
else()
    find_package(Qt5 COMPONENTS Widgets Multimedia REQUIRED)
    set(QT_VERSION_MAJOR 5)
    set(QT_LIB Qt5::Widgets Qt5::Multimedia)
endif()

# 头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 源文件
set(SOURCES
    src/main.cpp
    src/logindialog.cpp
    src/mainwindow.cpp
    src/storage.cpp
    src/TaskManager.cpp
    src/user.cpp
    src/utils.cpp
    src/taskdialog.cpp
)

file(GLOB HEADERS
    include/*.h
    include/*.hpp
)

# UI文件
file(GLOB UIS
    ui/*.ui
)

# 生成可执行文件
add_executable(final_work
    ${SOURCES}
    ${HEADERS}
    ${UIS}
)

target_link_libraries(final_work PRIVATE
    ${QT_LIB}
)
