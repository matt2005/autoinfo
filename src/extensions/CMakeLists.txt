cmake_minimum_required(VERSION 3.16)

set(EXTENSIONS_SOURCES
    extension_manifest.cpp
    extension_manager.cpp
)

set(EXTENSIONS_HEADERS
    extension.hpp
    extension_manifest.hpp
    extension_manager.hpp
)

add_library(CrankshaftExtensions STATIC
    ${EXTENSIONS_SOURCES}
    ${EXTENSIONS_HEADERS}
)

target_link_libraries(CrankshaftExtensions
    PUBLIC
        Qt6::Core
        CrankshaftCore
)

target_include_directories(CrankshaftExtensions
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

# Install headers
install(FILES ${EXTENSIONS_HEADERS}
    DESTINATION include/crankshaft/extensions
)
