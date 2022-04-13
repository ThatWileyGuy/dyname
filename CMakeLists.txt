cmake_minimum_required(VERSION 3.13)

project(DynamicName)

set(CMAKE_CXX_STANDARD 20)

# code
find_package(Boost REQUIRED unit_test_framework json)
find_package(cpr REQUIRED)
find_package(yaml-cpp REQUIRED)

file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/src/*.hxx)
file(GLOB_RECURSE SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.cxx)

add_library(dyname_lib OBJECT ${HEADER_FILES} ${SOURCE_FILES})
target_include_directories(dyname_lib PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(dyname_lib PRIVATE ${libssh_INCLUDE_DIRS})

#executable
add_executable(dyname main.cxx)
target_include_directories(dyname PRIVATE ${yaml-cpp_INCLUDE_DIRS})
target_link_libraries(dyname dyname_lib)
target_link_libraries(dyname cpr::cpr)
target_link_libraries(dyname Boost::json)
target_link_libraries(dyname yaml-cpp)


if(UNIX)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(dyname Threads::Threads)
target_link_libraries(dyname ${CMAKE_DL_LIBS})
set_target_properties(dyname PROPERTIES ENABLE_EXPORTS true)
endif()

# tests
file(GLOB TEST_HEADER_FILES ${PROJECT_SOURCE_DIR}/tests/*.hxx)
file(GLOB TEST_SOURCE_FILES ${PROJECT_SOURCE_DIR}/tests/*.cxx)

add_executable(dyname_tests ${TEST_HEADER_FILES} ${TEST_SOURCE_FILES})
target_compile_definitions(dyname_tests PRIVATE "BOOST_TEST_DYN_LINK=1")
target_link_libraries(dyname_tests Boost::unit_test_framework)
target_link_libraries(dyname_tests dyname_lib)
target_link_libraries(dyname_tests cpr::cpr)
target_link_libraries(dyname_tests Boost::json)
target_include_directories(dyname_tests PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(dyname_tests PRIVATE src)

if(UNIX)
target_link_libraries(dyname_tests Threads::Threads)
target_link_libraries(dyname_tests ${CMAKE_DL_LIBS})
endif()

enable_testing()
add_test(RoteTests dyname_tests)

