find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(MediaPlayerExtension SHARED
    media_player_extension.cpp
    media_player_extension.hpp
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
)

target_include_directories(MediaPlayerExtension
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
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
