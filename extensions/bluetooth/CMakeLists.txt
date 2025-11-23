find_package(Qt6 REQUIRED COMPONENTS Core Bluetooth)

add_library(BluetoothExtension SHARED
    bluetooth_extension.cpp
    bluetooth_extension.hpp
)

# Ensure the built library name matches manifest entry_point (bluetooth.so)
set_target_properties(BluetoothExtension PROPERTIES 
    OUTPUT_NAME bluetooth
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/extensions/bluetooth
)

# Copy manifest.json to build directory for development
add_custom_command(TARGET BluetoothExtension POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/manifest.json
        ${CMAKE_BINARY_DIR}/extensions/bluetooth/manifest.json
    COMMENT "Copying bluetooth manifest to build directory"
)

target_link_libraries(BluetoothExtension
    PRIVATE
    Qt6::Core
    Qt6::Bluetooth
    CrankshaftExtensions
)

target_include_directories(BluetoothExtension
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

install(TARGETS BluetoothExtension
    LIBRARY DESTINATION lib/${PROJECT_NAME}/extensions/bluetooth
    RUNTIME DESTINATION lib/${PROJECT_NAME}/extensions/bluetooth
)

install(FILES manifest.json
    DESTINATION share/${PROJECT_NAME}/extensions/bluetooth
)

install(DIRECTORY assets/
    DESTINATION share/${PROJECT_NAME}/extensions/bluetooth/assets
)
