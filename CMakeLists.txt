cmake_minimum_required(VERSION 4.2)
project(fraktalized LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Widgets)

add_executable(fraktalized
    src/main.cpp
)

qt_add_qml_module(fraktalized
    URI fraktalized
    VERSION 1.0
    QML_FILES
        qml/main.qml
)

target_link_libraries(fraktalized PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Widgets
)