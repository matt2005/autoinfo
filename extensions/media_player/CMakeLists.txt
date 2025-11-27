find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(PkgConfig REQUIRED)

# GStreamer dependencies
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0>=1.0)
pkg_check_modules(GSTREAMER_APP REQUIRED gstreamer-app-1.0>=1.0)
pkg_check_modules(GSTREAMER_PBUTILS REQUIRED gstreamer-pbutils-1.0>=1.0)

add_library(MediaPlayerExtension SHARED
    media_player_extension.cpp
    media_player_extension.hpp
    IMediaEngine.hpp
    GStreamerEngine.cpp
    GStreamerEngine.hpp
)

# Ensure the built library name matches manifest entry_point (media_player.so)
set_target_properties(MediaPlayerExtension PROPERTIES 
    OUTPUT_NAME media_player
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/extensions/media_player
)

# Copy manifest.json to build directory for development
add_custom_command(TARGET MediaPlayerExtension POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/manifest.json
        ${CMAKE_BINARY_DIR}/extensions/media_player/manifest.json
    COMMENT "Copying media_player manifest to build directory"
)

target_link_libraries(MediaPlayerExtension
    PRIVATE
    Qt6::Core
    CrankshaftExtensions
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER_APP_LIBRARIES}
    ${GSTREAMER_PBUTILS_LIBRARIES}
)

target_include_directories(MediaPlayerExtension
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_APP_INCLUDE_DIRS}
    ${GSTREAMER_PBUTILS_INCLUDE_DIRS}
)

install(TARGETS MediaPlayerExtension
    LIBRARY DESTINATION lib/${PROJECT_NAME}/extensions/media_player
    RUNTIME DESTINATION lib/${PROJECT_NAME}/extensions/media_player
)

install(FILES manifest.json
    DESTINATION share/${PROJECT_NAME}/extensions/media_player
)

install(DIRECTORY assets/
    DESTINATION share/${PROJECT_NAME}/extensions/media_player/assets
)
