# Extensions build configuration

if(BUILD_EXTENSIONS)
    message(STATUS "Building extensions...")
    
    # Add each extension subdirectory
    add_subdirectory(media_player)
    add_subdirectory(navigation)
    add_subdirectory(bluetooth)
    
    message(STATUS "Extensions configured successfully")
endif()
