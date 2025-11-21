find_package(Qt6 REQUIRED COMPONENTS Core Positioning Network)

# Add QML resources
set(NAVIGATION_QML_RESOURCES
    resources.qrc
)

# Geocoding provider sources
set(PROVIDER_SOURCES
    GeocodingProvider.hpp
    GeocodingProviderFactory.hpp
    GeocodingProviderFactory.cpp
    NominatimProvider.hpp
    NominatimProvider.cpp
)

add_library(NavigationExtension SHARED
    navigation_extension.cpp
    navigation_extension.hpp
    ${PROVIDER_SOURCES}
    ${NAVIGATION_QML_RESOURCES}
)

# Ensure the built library name matches manifest entry_point (navigation.so)
set_target_properties(NavigationExtension PROPERTIES 
    OUTPUT_NAME navigation
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/extensions/navigation
)

# Copy manifest.json to build directory for development
add_custom_command(TARGET NavigationExtension POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/manifest.json
        ${CMAKE_BINARY_DIR}/extensions/navigation/manifest.json
    COMMENT "Copying navigation manifest to build directory"
)

# Copy QML files to build directory for development
add_custom_command(TARGET NavigationExtension POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/qml
        ${CMAKE_BINARY_DIR}/extensions/navigation/qml
    COMMENT "Copying navigation QML files to build directory"
)

target_link_libraries(NavigationExtension
    PRIVATE
    Qt6::Core
    Qt6::Positioning
    Qt6::Network
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

install(DIRECTORY qml/
    DESTINATION share/${PROJECT_NAME}/extensions/navigation/qml
)
