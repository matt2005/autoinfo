find_package(Qt6 REQUIRED COMPONENTS Core Multimedia)

add_library(MediaPlayerExtension SHARED
    media_player_extension.cpp
    media_player_extension.hpp
)

# Ensure the built library name matches manifest entry_point (media_player.so)
set_target_properties(MediaPlayerExtension PROPERTIES OUTPUT_NAME media_player)

target_link_libraries(MediaPlayerExtension
    PRIVATE
    Qt6::Core
    Qt6::Multimedia
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
