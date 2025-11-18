find_package(Qt6 REQUIRED COMPONENTS Core Bluetooth)

add_library(BluetoothExtension SHARED
    bluetooth_extension.cpp
    bluetooth_extension.hpp
)

# Ensure the built library name matches manifest entry_point (bluetooth.so)
set_target_properties(BluetoothExtension PROPERTIES OUTPUT_NAME bluetooth)

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
