find_package(Qt6 REQUIRED COMPONENTS Core Bluetooth)

add_library(BluetoothExtension SHARED
    bluetooth_extension.cpp
    bluetooth_extension.hpp
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
    LIBRARY DESTINATION lib/crankshaft/extensions/bluetooth
    RUNTIME DESTINATION lib/crankshaft/extensions/bluetooth
)

install(FILES manifest.json
    DESTINATION lib/crankshaft/extensions/bluetooth
)

install(DIRECTORY assets/
    DESTINATION lib/crankshaft/extensions/bluetooth/assets
)
