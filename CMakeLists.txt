cmake_minimum_required(VERSION 2.8)

# set the project name
project(clusterer)

# add the executable
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS " -lstdc++fs")
set(CMAKE_BUILD_TYPE "Release")

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.71.0 COMPONENTS iostreams) 
find_library(STDCPPFS_LIBRARY NAMES stdc++fs)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
endif()

#AUX_SOURCE_DIRECTORY(./src SOURCES)
file(GLOB_RECURSE SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE LIB_SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.cpp
)
#add_library(clusterer_lib ${SOURCES} ${HEADERS})
add_executable(clusterer ${SOURCES})
target_include_directories(clusterer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

#clusterer executable 

set_target_properties(clusterer PROPERTIES RUNTIME_OUTPUT_DIRECTORY "./bin")
set_target_properties(clusterer PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "./bin")
set_target_properties(clusterer PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "./bin")
