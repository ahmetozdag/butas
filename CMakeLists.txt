cmake_minimum_required(VERSION 3.16)
project(butas LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

qt_add_executable(butas
    app/src/main.cpp
    app/src/theme.h
    app/src/icons.h
    app/src/blur.h
    app/src/format.h
    app/src/linedit.h
    app/src/widgets.h
    app/src/panel.h
    app/src/modals.h
    app/src/modal_boxes.h
    app/src/mainwindow.h
    app/resources.qrc
)

set_target_properties(butas PROPERTIES
    QT_ANDROID_PACKAGE_NAME "org.butas.app"
    QT_ANDROID_VERSION_NAME "1.0"
    QT_ANDROID_VERSION_CODE 3
    QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
)

target_include_directories(butas PRIVATE app/src)

target_link_libraries(butas PRIVATE Qt6::Widgets)
