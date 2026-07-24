cmake_minimum_required(VERSION 3.16)

project(final_work VERSION 1.0 LANGUAGES CXX)

# 使用 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 自动处理 moc / uic / rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 让 CMake 能找到 ui 目录里的 .ui 文件
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

# =========================
# 查找 Qt
# 优先 Qt6，如果没有 Qt6，就使用 Qt5
# =========================
find_package(Qt6 COMPONENTS Widgets QUIET)

if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    set(QT_LIB Qt6::Widgets)
else()
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    set(QT_VERSION_MAJOR 5)
    set(QT_LIB Qt5::Widgets)
endif()

# =========================
# 头文件目录
# =========================
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# =========================
# 源文件
# 你的 cpp 文件都在 src 目录下
# =========================
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

# =========================
# 头文件
# 如果 include 目录里的文件名和这里不一样，可以删掉或改名
# =========================
file(GLOB HEADERS
    include/*.h
    include/*.hpp
)

# =========================
# UI 文件
# 自动加入 ui 目录下所有 .ui 文件
# =========================
file(GLOB UIS
    ui/*.ui
)

# =========================
# 生成可执行文件
# =========================
add_executable(final_work
    ${SOURCES}
    ${HEADERS}
    ${UIS}
)

# 链接 Qt Widgets
target_link_libraries(final_work PRIVATE
    ${QT_LIB}
)
