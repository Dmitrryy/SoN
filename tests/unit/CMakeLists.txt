cpmaddpackage(
  NAME
  googletest
  GITHUB_REPOSITORY
  google/googletest
  GIT_TAG
  release-1.12.1
  VERSION
  1.12.1
  OPTIONS
  "INSTALL_GTEST OFF")
# find_package(GTest REQUIRED)

list_dirs(UNIT_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

set(TESTLIST)

macro(son_add_utest src)
  set(TEST_NAME "${src}_test")
  add_executable(${TEST_NAME} ${src})
  upd_tar_list(${TEST_NAME} TESTLIST)
endmacro()

foreach(DIR ${UNIT_DIRS})
  add_subdirectory(${DIR})
endforeach()

message(STATUS "Collected unit tests: ${TESTLIST}")
message(STATUS "Collected libs: ${LIBLIST}")

foreach(TEST ${TESTLIST})
  target_include_directories(${TEST} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(${TEST} PRIVATE GTest::gtest GTest::gtest_main pthread)
  target_link_libraries(${TEST} PRIVATE ${LIBLIST})

  target_compile_features(${TEST} PRIVATE cxx_std_20)
  target_include_directories(${TEST} PRIVATE ${CMAKE_SOURCE_DIR}/src)
  target_include_directories(${TEST} PRIVATE ${CMAKE_SOURCE_DIR}/tests/assets)

  add_test(NAME ${TEST} COMMAND ${TEST} --gtest_color=yes)
  set_property(TEST ${TEST} PROPERTY LABELS unit)
endforeach()