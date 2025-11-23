cmake_minimum_required(VERSION 3.16)
project(CrankshaftReborn VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Ensure all targets (including static libs) are built with position-independent code
# This avoids linker errors when linking static libraries into shared objects (.so)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Use Debian-friendly install prefix by default
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "" FORCE)
endif()

# Find Qt6 packages (include Bluetooth for capability; add Test when BUILD_TESTS)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml Widgets WebSockets Network Positioning Bluetooth)

if(BUILD_TESTS)
    find_package(Qt6 REQUIRED COMPONENTS Test)
endif()

# Project version
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)

# Build options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXTENSIONS "Build extensions" ON)
option(ENABLE_EGLFS "Enable EGLFS platform support" ON)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}
)

# Core library
add_subdirectory(src/core)

# UI library
add_subdirectory(src/ui)

# Extensions framework
if(BUILD_EXTENSIONS)
    add_subdirectory(src/extensions)
    # Build actual extension implementations
    add_subdirectory(extensions)
endif()

# Main application
set(MAIN_SOURCES
    src/main.cpp
)

add_executable(${PROJECT_NAME} ${MAIN_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::Widgets
        Qt6::WebSockets
        Qt6::Network
        Qt6::Positioning
        CrankshaftCore
        CrankshaftUI
        CrankshaftExtensions
        NavigationExtension
        BluetoothExtension
        MediaPlayerExtension
)

# Copy assets to build directory for development
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets
    COMMENT "Copying assets to build directory"
)

# Copy config to build directory for development
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/config
        ${CMAKE_BINARY_DIR}/config
    COMMENT "Copying config to build directory"
)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install targets
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Extension manifests/assets are installed by each extension's own CMakeLists

install(DIRECTORY config/
    DESTINATION etc/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.conf" PATTERN "*.json"
)

# Install theme assets
install(DIRECTORY assets/themes/
    DESTINATION share/${PROJECT_NAME}/themes
    FILES_MATCHING PATTERN "*.json"
)

# Install QML files
install(DIRECTORY assets/qml/
    DESTINATION share/${PROJECT_NAME}/qml
    FILES_MATCHING PATTERN "*.qml" PATTERN "qmldir"
)

# Package information
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_VENDOR "OpenCarDev Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Automotive infotainment system")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Debian package settings
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "OpenCarDev Team")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/matt2005/autoinfo")
# Enable automatic runtime dependency detection via dpkg-shlibdeps
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
# Include architecture and version in filename
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

include(CPack)

# CPack / DEB packaging configuration
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "crankshaft-reborn")
set(CPACK_PACKAGE_CONTACT "OpenCarDev Team <maintainers@opencardev.org>")
set(CPACK_PACKAGE_VENDOR "OpenCarDev Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Automotive infotainment system with extension framework")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
# Let dpkg-shlibdeps compute runtime dependencies from linked shared libs
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
# Additional runtime dependencies (QML modules not detected by shlibdeps)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qml6-module-qtquick, qml6-module-qtquick-controls, qml6-module-qtquick-layouts, qml6-module-qtquick-window, qml6-module-qtqml-workerscript, libqt6qml6, libqt6quick6, qt6-qpa-plugins, libqt6bluetooth6, bluez")
# Include license/readme files
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

