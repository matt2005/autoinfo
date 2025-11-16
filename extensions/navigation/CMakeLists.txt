find_package(Qt6 REQUIRED COMPONENTS Core Positioning)

add_library(NavigationExtension SHARED
    navigation_extension.cpp
    navigation_extension.hpp
)

target_link_libraries(NavigationExtension
    PRIVATE
    Qt6::Core
    Qt6::Positioning
    CrankshaftExtensions
)

target_include_directories(NavigationExtension
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

install(TARGETS NavigationExtension
    LIBRARY DESTINATION lib/crankshaft/extensions/navigation
    RUNTIME DESTINATION lib/crankshaft/extensions/navigation
)

install(FILES manifest.json
    DESTINATION lib/crankshaft/extensions/navigation
)

install(DIRECTORY assets/
    DESTINATION lib/crankshaft/extensions/navigation/assets
)
