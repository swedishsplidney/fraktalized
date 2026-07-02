cmake_minimum_required(VERSION 3.20)
project(fraktalized LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(APPLE)
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/mock_frameworks/AGL.framework")

    file(WRITE "${CMAKE_BINARY_DIR}/agl_stub.c" "void dummy_agl_symbol(void) {}\n")

    execute_process(
            COMMAND clang -x c "${CMAKE_BINARY_DIR}/agl_stub.c" -shared -o "${CMAKE_BINARY_DIR}/mock_frameworks/AGL.framework/AGL"
            RESULT_VARIABLE CLANG_STUB_RESULT
    )

    set(OPENGL_agl_LIBRARY "${CMAKE_BINARY_DIR}/mock_frameworks/AGL.framework/AGL" CACHE FILEPATH "" FORCE)
    set(WrapOpenGL_FOUND TRUE CACHE BOOL "" FORCE)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Widgets OpenGL)

add_executable(${PROJECT_NAME} MACOSX_BUNDLE
        src/main.cpp
        src/fractalengine.h
        src/fractalengine.cpp
        src/tutorialmanager.h
        src/tutorialmanager.cpp
)

if(APPLE)
    target_link_options(${PROJECT_NAME} PRIVATE "-F${CMAKE_BINARY_DIR}/mock_frameworks")
endif()

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
        shaders/trajectory.glsl
        shaders/ifs.glsl
        shaders/3d.vert
        shaders/3d.frag
        shaders/menger.frag
        shaders/icebox.frag
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::Widgets
)