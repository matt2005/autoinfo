find_package(Qt6 REQUIRED COMPONENTS Core Multimedia)

add_library(MediaPlayerExtension SHARED
    media_player_extension.cpp
    media_player_extension.hpp
)

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
    LIBRARY DESTINATION lib/crankshaft/extensions/media_player
    RUNTIME DESTINATION lib/crankshaft/extensions/media_player
)

install(FILES manifest.json
    DESTINATION lib/crankshaft/extensions/media_player
)

install(DIRECTORY assets/
    DESTINATION lib/crankshaft/extensions/media_player/assets
)
