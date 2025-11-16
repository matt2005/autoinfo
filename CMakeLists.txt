cmake_minimum_required(VERSION 3.16)
project(CrankshaftReborn VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Widgets WebSockets Network)

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
        CrankshaftCore
        CrankshaftExtensions
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

install(DIRECTORY extensions/
    DESTINATION share/${PROJECT_NAME}/extensions
    FILES_MATCHING PATTERN "*.json"
)

install(DIRECTORY config/
    DESTINATION etc/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.conf" PATTERN "*.json"
)

# Package information
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_VENDOR "OpenCarDev Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Automotive infotainment system")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

include(CPack)
