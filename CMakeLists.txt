cmake_minimum_required(VERSION 3.2)
project(fhe)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(ENABLE_TEST "Build the test files" ON)
option(ENABLE_THREADS "To use multi-thread. Make Sure the NTL is built with NTL_THREADS=on" ON)
option(BUILD_SHARED "To build shared objects" OFF)
option(BUILD_AES "To build homomorphic AES" OFF)

## Might modify the following path to NTL according to your environment
set(NTL_HEADER /home/nlukas/anaconda3/envs/psi_helib/include)
set(NTL_LIB /home/nlukas/anaconda3/envs/psi_helib/lib)
include_directories(${NTL_HEADER})
link_directories(${NTL_LIB})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread -lcrypto")
## Use -std=c++11 as dedeault. 
set(CMAKE_CXX_STANDARD 17)
## On old platform, might change to -std=c++0x
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

## To use multi-threads.
## Make sure the NTL is built with NTL_THREADS=on
## For NTL's verison older than 7.0, should turn FHE_THREADS off.
if (ENABLE_THREADS)
  set(FHE_THREADS ON) 
  add_definitions(-DFHE_THREADS)
  set(RUN_LIB fhe ntl gmp pthread)
else (ENABLE_THREADS)
  set(FHE_THREADS OFF) 
  set(RUN_LIB fhe ntl gmp)
endif (ENABLE_THREADS)

# Use Armadillo if you can find it
find_package(Armadillo)
if (ARMADILLO_FOUND)
  include_directories(${ARMADILLO_INCLUDE_DIRS})
  link_libraries(${ARMADILLO_LIBRARIES})
  add_definitions(-DFFT_ARMA)
else (ARMADILLO_FOUND)
  add_definitions(-DFFT_NATIVE)
endif (ARMADILLO_FOUND)

find_package(OpenSSL)
if(OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIRS})
  link_libraries(${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)

add_subdirectory(src)
