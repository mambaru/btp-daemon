project(btp CXX C)

cmake_minimum_required(VERSION 2.6)

#ENABLE_TESTING(true)
SET(CMAKE_CXX_COMPILER "g++46")
execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
if (NOT (GCC_VERSION VERSION_GREATER 4.5 OR GCC_VERSION VERSION_EQUAL 4.5))
	message(FATAL_ERROR "GCC 4.5+ Required")
endif()

SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb -fno-inline ")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2 -ggdb -march=core2")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb ")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -ggdb -march=core2")

SET(CMAKE_CXX_FLAGS "-Wall -DHAVE_CONFIG_H --std=c++0x")
#-I../hitlist/include")
SET(CMAKE_C_FLAGS "-Wall")

include_directories(.)

add_executable(daemon src/daemon.cpp common/params.cpp)

# Если вдруг boost_thread-mt не будет найдена, попробуйте заменить на boost_thread  - зависит от версии буста
target_link_libraries(daemon pthread boost_program_options z tbb boost_thread kyotocabinet)

#set_property(TARGET daemon APPEND PROPERTY LINK_FLAGS "-L/home/shepik/misc/lib/")

#target_link_libraries(test kyotocabinet)
