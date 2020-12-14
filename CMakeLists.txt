cmake_minimum_required(VERSION 3.14)
project(lightdb)

include_directories("${PROJECT_SOURCE_DIR}/includes/")
include_directories("${PROJECT_SOURCE_DIR}")
set(CMAKE_THREAD_LIBS_INIT "-lpthread")

set(CMAKE_CXX_STANDARD 14)

file(GLOB_RECURSE SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/db/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/*.cpp 
)

add_executable(lightdb lightdb.cpp ${SOURCE})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(lightdb PRIVATE Threads::Threads)






