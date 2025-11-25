cmake_minimum_required(VERSION 3.16)

set(WIRELESS_SOURCES
    wireless_extension.cpp
)

set(WIRELESS_HEADERS
    wireless_extension.hpp
)

# Enable Qt resource compilation
set(CMAKE_AUTORCC ON)

add_library(WirelessExtension STATIC
    ${WIRELESS_SOURCES}
    ${WIRELESS_HEADERS}
    resources.qrc
)

# Give the QRC a unique resource name to avoid collisions and allow Q_INIT_RESOURCE
set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc
    PROPERTIES AUTORCC_OPTIONS "-name;wireless_resources"
)

target_link_libraries(WirelessExtension
    PUBLIC
        Qt6::Core
        Qt6::DBus
        CrankshaftCore
        CrankshaftExtensions
)

target_include_directories(WirelessExtension
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

# Copy manifest.json to build directory for development
add_custom_command(TARGET WirelessExtension POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/manifest.json
        ${CMAKE_BINARY_DIR}/extensions/wireless/manifest.json
    COMMENT "Copying wireless manifest to build directory"
)

# Copy QML files to build directory for development
add_custom_command(TARGET WirelessExtension POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/extensions/wireless/assets
    COMMENT "Copying wireless assets to build directory"
)

# Install extension files
install(FILES manifest.json
    DESTINATION share/${CMAKE_PROJECT_NAME}/extensions/wireless
)

install(DIRECTORY assets/
    DESTINATION share/${CMAKE_PROJECT_NAME}/extensions/wireless/assets
    FILES_MATCHING PATTERN "*.qml" PATTERN "*.js" PATTERN "*.png" PATTERN "*.svg"
)
