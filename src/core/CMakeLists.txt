cmake_minimum_required(VERSION 3.16)

set(CORE_SOURCES
    application/application.cpp
    events/event_bus.cpp
    network/websocket_server.cpp
    capabilities/CapabilityManager.cpp
    capabilities/BluetoothCapability.cpp
    capabilities/WirelessCapabilityImpl.cpp
    config/ConfigManager.cpp
    config/ConfigTypes.cpp
)

set(CORE_HEADERS
    application/application.hpp
    events/event_bus.hpp
    network/websocket_server.hpp
    capabilities/CapabilityManager.hpp
    config/ConfigManager.hpp
    config/ConfigTypes.hpp
    ui/UIRegistrar.hpp
    capabilities/Capability.hpp
    capabilities/LocationCapability.hpp
    capabilities/NetworkCapability.hpp
    capabilities/FileSystemCapability.hpp
    capabilities/UICapability.hpp
    capabilities/EventCapability.hpp
    capabilities/BluetoothCapability.hpp
    capabilities/WirelessCapability.hpp
    capabilities/WirelessCapabilityImpl.hpp
    capabilities/AudioCapability.hpp
    capabilities/VideoCapability.hpp
)

add_library(CrankshaftCore STATIC
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

target_link_libraries(CrankshaftCore
    PUBLIC
        Qt6::Core
        Qt6::WebSockets
        Qt6::Network
        Qt6::Positioning
        Qt6::Qml
        Qt6::Bluetooth
)

target_include_directories(CrankshaftCore
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

# Install headers
install(FILES ${CORE_HEADERS}
    DESTINATION include/crankshaft/core
)
