cmake_minimum_required(VERSION 3.16)
project(BPlusTree)

set(CMAKE_CXX_STANDARD 14)

add_executable(code
        src/main.cpp
        src/utility/bpt.hpp
        src/utility/vector.hpp
        src/utility/exception.hpp
        src/head-file/key.hpp
        src/head-file/value.hpp
        src/utility/file_manager.hpp
        #        pai/try.cpp
        src/utility/BPlusTree.hpp)
