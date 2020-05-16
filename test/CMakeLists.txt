find_package(GTest REQUIRED)

add_executable(diffpptest main.cc)

target_link_libraries(diffpptest PRIVATE diffpp-headers GTest::GTest GTest::Main)

gtest_discover_tests(diffpptest)
