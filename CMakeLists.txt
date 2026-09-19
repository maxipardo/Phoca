cmake_minimum_required(VERSION 3.16)
project(Phoca VERSION 0.5)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets Core Network LinguistTools REQUIRED)

add_executable(phoca
    WIN32
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/Service.cpp
    src/Service.h
    src/ServiceMaintainer.cpp
    src/ServiceMaintainer.h
    src/resources.qrc
    src/About.h
    src/About.cpp
    src/DownloadItem.h
    src/DownloadItem.cpp
    src/DownloadConfig.h
    src/DownloadError.h
)

if(WIN32)
    target_sources(phoca PRIVATE data/icon.rc)
endif()

target_link_libraries(phoca PRIVATE Qt6::Widgets Qt6::Core Qt6::Network)

if(UNIX AND NOT APPLE)
    find_package(Qt6 REQUIRED COMPONENTS DBus)
    target_link_libraries(phoca PRIVATE Qt6::DBus)
endif()

set(TS_FILES i18n/phoca_es.ts)
qt_add_translations(phoca TS_FILES ${TS_FILES})

target_compile_definitions(phoca PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")

# Flatpak icon
option(FLATPAK_BUILD "Set names for Flatpak" OFF)

if(FLATPAK_BUILD)
    set(APP_ID "io.github.maxipardo.phoca")
else()
    set(APP_ID "phoca")
endif()

target_compile_definitions(phoca PRIVATE DESKTOP_FILE_NAME="${APP_ID}")

# ==========================================
# Installation
# ==========================================

install(TARGETS phoca DESTINATION bin)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ext_bin/deno")
    install(PROGRAMS ext_bin/deno DESTINATION bin)
endif()

configure_file(data/phoca.desktop.in phoca.desktop @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/phoca.desktop
        DESTINATION share/applications
        RENAME ${APP_ID}.desktop)

install(FILES data/phoca256.png 
        DESTINATION share/icons/hicolor/256x256/apps
        RENAME ${APP_ID}.png)

install(FILES data/io.github.maxipardo.phoca.metainfo.xml
        DESTINATION share/metainfo)

# ==========================================
# CPack (Debian packaging)
# ==========================================

set(CPACK_GENERATOR "DEB")
# Package info
set(CPACK_PACKAGE_NAME "phoca")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "Maximo Pardo")
set(CPACK_PACKAGE_CONTACT "Maximo Pardo <maxipardo@proton.me>")
# Title and category
set(CPACK_DEBIAN_PACKAGE_NAME "Phoca")
set(CPACK_DEBIAN_PACKAGE_SECTION "utility")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/maxipardo/Phoca")

# Descriptions
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Download any video/audio through yt-dlp")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Phoca is a GUI app for yt-dlp to allow for simple and easy downloads from most internet services like YouTube and other social media.\n\nIt supports playlists, audio extraction, format conversion and saving thumbnails.")

# LICENSE
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "ffmpeg")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)