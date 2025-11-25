cmake_minimum_required(VERSION 3.16)

set(WIRELESS_SOURCES
    wireless_extension.cpp
)

set(WIRELESS_HEADERS
    wireless_extension.hpp
)

add_library(WirelessExtension STATIC
    ${WIRELESS_SOURCES}
    ${WIRELESS_HEADERS}
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

# Add QRC resources to expose QML via qrc:/wireless/qml/WirelessView.qml
# Properly add QRC resources (resources.qrc already defines prefix /wireless)
qt_add_resources(WirelessExtension "wireless_resources"
    FILES
        resources.qrc
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
