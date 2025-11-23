find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(DialerExtension SHARED
    dialer_extension.cpp
    dialer_extension.hpp
    resources.qrc
)

# Ensure the built library name matches manifest entry_point (dialer.so)
set_target_properties(DialerExtension PROPERTIES 
    OUTPUT_NAME dialer
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/extensions/dialer
)

# Copy manifest.json to build directory for development
add_custom_command(TARGET DialerExtension POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/manifest.json
        ${CMAKE_BINARY_DIR}/extensions/dialer/manifest.json
    COMMENT "Copying dialer manifest to build directory"
)

# Copy QML files to build directory for development
add_custom_command(TARGET DialerExtension POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/qml
        ${CMAKE_BINARY_DIR}/extensions/dialer/qml
    COMMENT "Copying dialer QML files to build directory"
)

target_link_libraries(DialerExtension
    PRIVATE
    Qt6::Core
    CrankshaftExtensions
)

target_include_directories(DialerExtension
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

install(TARGETS DialerExtension
    LIBRARY DESTINATION lib/${PROJECT_NAME}/extensions/dialer
    RUNTIME DESTINATION lib/${PROJECT_NAME}/extensions/dialer
)

install(FILES manifest.json
    DESTINATION share/${PROJECT_NAME}/extensions/dialer
)

install(DIRECTORY assets/
    DESTINATION share/${PROJECT_NAME}/extensions/dialer/assets
)

install(DIRECTORY qml/
    DESTINATION share/${PROJECT_NAME}/extensions/dialer/qml
)
