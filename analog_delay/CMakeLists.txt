cmake_minimum_required(VERSION 3.0)

project(FirstSystemCProject)

set(CMAKE_CXX_STANDARD 17)            # Options: 98, 11, 14, 17, 20, 23, 26
set(CMAKE_CXX_STANDARD_REQUIRED ON)   # Stops build if standard isn't supported
#set(CMAKE_CXX_EXTENSIONS OFF)         # Use standard C++ (no GNU/MSVC extensions)

# Set the SystemC include and library directories
set(SYSTEMC_INCLUDE_DIR $ENV{SYSTEMC_HOME}/include)
set(SYSTEMC_LIBRARY_DIR $ENV{SYSTEMC_HOME}/lib)

set(SYSTEMCAMS_INCLUDE_DIR $ENV{SYSTEMC_AMS_HOME}/include)
set(SYSTEMCAMS_LIBRARY_DIR $ENV{SYSTEMC_AMS_HOME}/lib)

# Add the executable
add_executable(first_systemc main.cpp)

# Link against SystemC
target_include_directories(first_systemc PRIVATE ${SYSTEMC_INCLUDE_DIR})
target_include_directories(first_systemc PRIVATE ${SYSTEMCAMS_INCLUDE_DIR})

target_link_libraries(first_systemc ${SYSTEMC_LIBRARY_DIR}/libsystemc.so)
target_link_libraries(first_systemc ${SYSTEMCAMS_LIBRARY_DIR}/libsystemc-ams.a)
