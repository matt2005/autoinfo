cmake_minimum_required(VERSION 3.16)

set(UI_SOURCES
    Theme.cpp
    ThemeManager.cpp
    ExtensionRegistry.cpp
    NavigationBridge.cpp
)

set(UI_HEADERS
    Theme.hpp
    ThemeManager.hpp
    ExtensionRegistry.hpp
    NavigationBridge.hpp
)

add_library(CrankshaftUI STATIC
    ${UI_SOURCES}
    ${UI_HEADERS}
)

target_link_libraries(CrankshaftUI
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Network
        CrankshaftExtensions
)

target_include_directories(CrankshaftUI
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

# Install headers
install(FILES ${UI_HEADERS}
    DESTINATION include/crankshaft/ui
)
