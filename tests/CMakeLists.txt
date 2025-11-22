cmake_minimum_required(VERSION 3.16)

enable_testing()

add_executable(test_extension_dependencies unit/test_extension_dependencies.cpp)
# Ensure static lib resolution: place UI after Core
target_link_libraries(test_extension_dependencies
	Qt6::Core Qt6::Qml Qt6::Test
	CrankshaftCore
	CrankshaftUI
	CrankshaftExtensions
)
add_test(NAME test_extension_dependencies COMMAND test_extension_dependencies)
