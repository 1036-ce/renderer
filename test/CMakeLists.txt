# find GTest lib
find_package(GTest REQUIRED)

# GTest header file
include_directories(${GTEST_INCLUDE_DIRS})

file(GLOB SRC ../src/*)
file(GLOB TEST ./test/*)

add_executable(tgaimage_test ${SRC} ${TEST})

target_link_libraries(tgaimage_test
        ${GTEST_BOTH_LIBRARIES}
        pthread )

# add to test
gtest_discover_tests(tgaimage_test)