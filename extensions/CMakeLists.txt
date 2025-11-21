# Extensions build configuration

if(BUILD_EXTENSIONS)
    message(STATUS "Building extensions...")
    
    # Add each extension subdirectory
    # NOTE: Only navigation uses capability-based architecture currently
    # add_subdirectory(media_player)  # TODO: Convert to capability-based
    add_subdirectory(navigation)
    # add_subdirectory(bluetooth)  # TODO: Convert to capability-based
    
    message(STATUS "Extensions configured successfully")
endif()
