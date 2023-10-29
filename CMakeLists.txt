cmake_minimum_required(VERSION 3.16)


project(son_ir)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

include(cmake/list_dirs.cmake)
include(cmake/upd_tar_list.cmake)
include(cmake/CPM.cmake)
include(cmake/comp_flags.cmake)

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
