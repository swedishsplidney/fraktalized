cmake_minimum_required(VERSION 4.2)
project(fraktalized LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(APPLE)
    add_library(WrapOpenGL::AGL INTERFACE IMPORTED)
    set_target_properties(WrapOpenGL::AGL PROPERTIES INTERFACE_LINK_LIBRARIES "")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Widgets OpenGL)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/fractalengine.h
    src/fractalengine.cpp
    src/tutorialmanager.h
    src/tutorialmanager.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

qt_add_qml_module(${PROJECT_NAME}
    URI fraktalizedModule
    VERSION 1.0
    QML_FILES
        qml/main.qml
    SOURCES
        src/fractalengine.h
        src/fractalengine.cpp
    RESOURCES
        shaders/fractal.vert
        shaders/fractal.frag
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Widgets
)