cmake_minimum_required(VERSION 3.16)
project(Phoca VERSION 0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets Core Network REQUIRED)

add_executable(phoca
    WIN32
    main.cpp
    MainWindow.cpp
    MainWindow.h
    Service.cpp
    Service.h
    ServiceMaintainer.cpp
    ServiceMaintainer.h
    resources.qrc
    About.h
    About.cpp
)

if(WIN32)
    target_sources(phoca PRIVATE icon.rc)
endif()

# Enlazamos las librerías de Qt al ejecutable
target_link_libraries(phoca PRIVATE Qt6::Widgets Qt6::Core Qt6::Network)
