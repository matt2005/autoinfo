cmake_minimum_required(VERSION 3.16)

set(CORE_SOURCES
    application.cpp
    event_bus.cpp
    websocket_server.cpp
    CapabilityManager.cpp
)

set(CORE_HEADERS
    application.hpp
    event_bus.hpp
    websocket_server.hpp
    CapabilityManager.hpp
    capabilities/Capability.hpp
    capabilities/LocationCapability.hpp
    capabilities/NetworkCapability.hpp
    capabilities/FileSystemCapability.hpp
    capabilities/UICapability.hpp
    capabilities/EventCapability.hpp
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
)

target_include_directories(CrankshaftCore
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

# Install headers
install(FILES ${CORE_HEADERS}
    DESTINATION include/crankshaft/core
)
