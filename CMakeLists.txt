CMAKE_MINIMUM_REQUIRED(VERSION 3.7)

project (NativeServiceArchitecture VERSION 0.1 LANGUAGES CXX)
set (CMAKE_PROJECT_NAME_SHORT "NSA")

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)

set (NSA_HEADERS
	"include/Service.hpp"
	"include/BlockingQueue.hpp"
)

set (UNITTEST_BLOCKINGQUEUE
	"unit/BlockingQueueTest.cpp"
)

set (NSA_SOURCES
	"src/dummy.cpp"
)

add_library(NativeServiceArchitecture ${NSA_SOURCES})

target_include_directories(NativeServiceArchitecture PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME_SHORT})

add_executable(unit_BlockingQueue ${UNITTEST_BLOCKINGQUEUE})

target_link_libraries(unit_BlockingQueue NativeServiceArchitecture pthread)
target_include_directories(unit_BlockingQueue PRIVATE include)

enable_testing()

add_test(unit_BlockingQueue unit_BlockingQueue)
