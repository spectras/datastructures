cmake_minimum_required(VERSION 3.0)
project(datastructures LANGUAGES CXX)

option(TESTS "build tests" on)

##############################################################################

add_compile_options(
    -Wall -Wextra
    -Wcast-align -Wcast-qual -Wold-style-cast
    -Wconversion -Wsign-conversion
    -Wshadow -Wstrict-overflow=3
)

add_library(datastructures INTERFACE)
target_include_directories(datastructures INTERFACE include/)
export(TARGETS datastructures NAMESPACE spectras:: FILE datastructures.cmake)

##############################################################################

if(TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    find_package(Threads REQUIRED)

    add_executable(rbtree tests/rbtree.cxx)
    target_compile_options(rbtree PRIVATE -std=c++17)
    target_include_directories(rbtree SYSTEM PRIVATE tests ${GTEST_INCLUDE_DIRS})
    target_link_libraries(rbtree datastructures ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    add_test(NAME rbtree COMMAND rbtree)

    add_executable(ringbuffer tests/RingBuffer.cxx)
    target_compile_options(ringbuffer PRIVATE -std=c++17)
    target_include_directories(ringbuffer SYSTEM PRIVATE tests ${GTEST_INCLUDE_DIRS})
    target_link_libraries(ringbuffer datastructures ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    add_test(NAME ringbuffer COMMAND ringbuffer)
endif(TESTS)
