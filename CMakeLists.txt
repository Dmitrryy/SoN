cmake_minimum_required(VERSION 3.10)


project(son_ir)

set(CMAKE_CXX_STANDARD 17)

add_executable(son
    main.cpp

    src/Node.cpp
)
