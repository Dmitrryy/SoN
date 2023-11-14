
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

add_library(sonIR STATIC 
Node.cpp
Function.cpp
)


target_include_directories(sonIR PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


set(LIBLIST
    sonIR
    PARENT_SCOPE)
