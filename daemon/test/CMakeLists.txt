include_directories(${gtest_SOURCE_DIR}/include)
set(test_sources
	# files containing the actual tests
	test.cpp
)
add_executable(tests ${test_sources})
target_link_libraries(tests gtest_main)