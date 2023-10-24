cmake_minimum_required(VERSION 3.16)


project(son_ir)

set(CMAKE_CXX_STANDARD 20)

include(cmake/list_dirs.cmake)
include(cmake/upd_tar_list.cmake)
include(cmake/CPM.cmake)
include(cmake/comp_flags.cmake)


enable_testing()
add_subdirectory(tests)
