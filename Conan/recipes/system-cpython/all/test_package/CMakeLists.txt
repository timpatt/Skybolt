cmake_minimum_required(VERSION 3.26)
project(SystemCPythonTestPackage LANGUAGES CXX)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

add_executable(system_cpython_embedding main.cpp)
target_link_libraries(system_cpython_embedding PRIVATE Python3::Python)
