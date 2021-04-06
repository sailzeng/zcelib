make_minimum_required(VERSION 3.10)

# set the project name
project(zcelib ERSION 1.0)

# add the executable
add_executable(Tutorial tutorial.cxx)

# C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)