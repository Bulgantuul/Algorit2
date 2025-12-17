cmake_minimum_required(VERSION 3.10)
project(Algorit)

set(CMAKE_CXX_STANDARD 17)

# 1. Find the Hunspell library package
find_package(PkgConfig REQUIRED)
pkg_check_modules(HUNSPELL REQUIRED hunspell)

# 2. Add the include directories for Hunspell
include_directories(${HUNSPELL_INCLUDE_DIRS})

# 3. FIX: Change 'main.cpp' to 'hunspell.cpp' here
add_executable(Algorit hunspell.cpp)

# 4. Link the libraries to your executable
target_link_libraries(Algorit ${HUNSPELL_LIBRARIES})