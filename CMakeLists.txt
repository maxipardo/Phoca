cmake_minimum_required(VERSION 3.16)
project(Phoca VERSION 0.2)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets Core Network LinguistTools REQUIRED)


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

target_link_libraries(phoca PRIVATE Qt6::Widgets Qt6::Core Qt6::Network)

set(TS_FILES i18n/phoca_es.ts)
qt_add_translations(phoca TS_FILES ${TS_FILES})

target_compile_definitions(phoca PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")