CMAKE_MINIMUM_REQUIRED(VERSION 3.7)

project (NativeServiceArchitecture)

set (Service_VERSION_MAJOR 0)
set (Service_VERSION_MINOR 0)

set (CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set (LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

set (UNITTEST_BLOCKINGQUEUE
	"src/BlockingQueueTest.cpp"
)

set (EXAMPLE_SERVICE
	"src/Example01.cpp"
)

add_executable(unit_BlockingQueue ${UNITTEST_BLOCKINGQUEUE})
add_executable(example01 ${EXAMPLE_SERVICE})
target_link_libraries(unit_BlockingQueue "pthread")
target_link_libraries(example01 "pthread")
