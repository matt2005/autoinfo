find_package(Qt6 REQUIRED COMPONENTS Core Positioning)

add_library(NavigationExtension SHARED
    navigation_extension.cpp
    navigation_extension.hpp
)

# Ensure the built library name matches manifest entry_point (navigation.so)
set_target_properties(NavigationExtension PROPERTIES OUTPUT_NAME navigation)

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
    LIBRARY DESTINATION lib/${PROJECT_NAME}/extensions/navigation
    RUNTIME DESTINATION lib/${PROJECT_NAME}/extensions/navigation
)

install(FILES manifest.json
    DESTINATION share/${PROJECT_NAME}/extensions/navigation
)

install(DIRECTORY assets/
    DESTINATION share/${PROJECT_NAME}/extensions/navigation/assets
)
