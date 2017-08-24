cmake_minimum_required (VERSION 2.8)
project (morph)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake_modules)

############################################
# find dependencies

find_package(Boost REQUIRED COMPONENTS filesystem iostreams system serialization regex)
include_directories(${Boost_INCLUDE_DIRS})

find_package(GSL REQUIRED COMPONENTS)
include_directories(${GSL_INCLUDE_DIR})

find_package(YAMLCpp REQUIRED COMPONENTS)
include_directories(${YAMLCPP_INCLUDE_DIR})

############################################

include_directories(BEFORE ${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")
set(CMAKE_CXX_FLAGS "-Wall -Wno-sign-compare -std=c++11 ${SDL_CFLAGS}")

file(GLOB_RECURSE Sources morphc/*.cpp)
add_executable(morph ${Sources})

target_link_libraries(morph
	${GSL_LIBRARIES}
	${Boost_LIBRARIES}
	${YAMLCPP_LIBRARY}
    ${LIBS})

