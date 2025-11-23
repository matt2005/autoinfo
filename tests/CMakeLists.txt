cmake_minimum_required(VERSION 3.16)

find_package(Qt6 REQUIRED COMPONENTS Test)

enable_testing()

# Test: Event Bus
add_executable(test_event_bus unit/test_event_bus.cpp)
target_link_libraries(test_event_bus
    Qt6::Core
    Qt6::Test
    CrankshaftCore
    CrankshaftExtensions
)
add_test(NAME test_event_bus COMMAND test_event_bus)

# Test: Extension Manager (without UI dependencies)
add_executable(test_extension_manager unit/test_extension_manager.cpp)
target_link_libraries(test_extension_manager
    Qt6::Core
    Qt6::Test
    CrankshaftCore
    CrankshaftExtensions
)
add_test(NAME test_extension_manager COMMAND test_extension_manager)

# Test: Config Manager integration
add_executable(test_config_manager integration/test_config_manager.cpp)
target_link_libraries(test_config_manager
    Qt6::Core
    Qt6::Test
    CrankshaftCore
    CrankshaftExtensions
)
add_test(NAME test_config_manager COMMAND test_config_manager)

# Test: Media public control events
add_executable(test_media_public_controls integration/test_media_public_controls.cpp)
target_link_libraries(test_media_public_controls
    Qt6::Core
    Qt6::Test
    CrankshaftCore
    CrankshaftExtensions
    MediaPlayerExtension
)
add_test(NAME test_media_public_controls COMMAND test_media_public_controls)
