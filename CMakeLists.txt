cmake_minimum_required(VERSION 3.11.3)

# set(CMAKE_CXX_STANDARD 17)
project(Crypto_real_time_monitor)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -lcurses -lcurl -pthread -lform -lssl -lcrypto")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/executable)
find_package(Curses REQUIRED)
find_package(OpenSSL REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

find_package(CURL REQUIRED) 
include_directories(${CURL_INCLUDE_DIR})

include_directories(include)

# Find all executables
file(GLOB project_SRCS src/*.cpp) #src/*.h
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/executable/data_test)

# Add project executable
add_executable(crypto_monitor ${project_SRCS})

set_property(TARGET crypto_monitor PROPERTY CXX_STANDARD 17)
target_link_libraries(crypto_monitor OpenSSL::SSL)
target_link_libraries(crypto_monitor ${CURSES_LIBRARIES})
target_link_libraries(crypto_monitor ${CURL_LIBRARIES})