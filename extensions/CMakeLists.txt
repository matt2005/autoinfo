# Extensions build configuration

if(BUILD_EXTENSIONS)
    message(STATUS "Building extensions...")
    
    # Add each extension subdirectory
    add_subdirectory(bluetooth)
    add_subdirectory(media_player)
    add_subdirectory(navigation)
    add_subdirectory(dialer)
    add_subdirectory(wireless)
    
    message(STATUS "Extensions configured successfully")
endif()
