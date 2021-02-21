cmake_minimum_required(VERSION 3.11.3)

# set(CMAKE_CXX_STANDARD 17)
project(Crypto_real_time_monitor)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -lcurses -lcurl -pthread -lform -lssl -lcrypto")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/executable)
find_package(Curses REQUIRED)
find_package(OpenSSL REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

find_package(CURL REQUIRED) 
include_directories(${CURL_INCLUDE_DIR})

# Find all executables
file(GLOB project_SRCS src/*.cpp) #src/*.h
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/executable/data_test)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/executable/data_aggregated)

# Set path where all monitor data will be going to
set(DATA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/executable/data_test/)
set(DATA_PATH_AGG ${CMAKE_CURRENT_SOURCE_DIR}/build/executable/data_aggregated/)

include_directories(include)
include_directories(Utils)

# Add project executable
add_executable(crypto_monitor ${project_SRCS})

# Pass DATA_PATH variable to SetPath.cmake
add_custom_command(TARGET crypto_monitor PRE_BUILD 
        COMMAND ${CMAKE_COMMAND} 
        -DDATA_PATH=${DATA_PATH} 
        -DDATA_PATH_AGG=${DATA_PATH_AGG} 
        -P ${CMAKE_CURRENT_SOURCE_DIR}/SetPath.cmake)

# Add runWallet script to import environmental variables        
FILE(WRITE ${CMAKE_BINARY_DIR}/runWallet.sh
"#!/bin/sh\n"

"for f in BINANCE_API_KEY BINANCE_API_SECRET; do\n" 

"export $f\n"
"done\n"
"source ~/.bashrc\n"

"${CMAKE_BINARY_DIR}/executable/crypto_monitor\n"
)

# Final install script, that will be installing cryptomonitor as a command
# to be run in the terminal
FILE(WRITE ${CMAKE_BINARY_DIR}/pre_install.sh
"#!/bin/sh\n"
"make\n"
)

FILE(WRITE ${CMAKE_BINARY_DIR}/install.sh
"#!/bin/sh\n"
"source ${CMAKE_BINARY_DIR}/pre_install.sh\n"
"source ${CMAKE_BINARY_DIR}/pre_install.sh\n"
"sudo mv ${CMAKE_BINARY_DIR}/runWallet.sh /usr/local/bin/cryptomonitor\n" 
"sudo chown root: /usr/local/bin/cryptomonitor\n"
"sudo chmod 755 /usr/local/bin/cryptomonitor\n"
)

set_property(TARGET crypto_monitor PROPERTY CXX_STANDARD 17)
target_link_libraries(crypto_monitor OpenSSL::SSL)
target_link_libraries(crypto_monitor ${CURSES_LIBRARIES})
target_link_libraries(crypto_monitor ${CURL_LIBRARIES})