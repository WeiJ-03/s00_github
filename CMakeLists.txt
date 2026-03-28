cmake_minimum_required(VERSION 3.14)
project(s000_model)


set(CMAKE_CXX_STANDARD 11)

#include_directories(Utility/source/module/head)

if (WIN32)
    Message("Windows system")
    if (MSVC)
    endif()
elseif(APPLE)
    Message("Apple Mac OS X system")
    set(ASAN_FLAGS "-fsanitize=address -fno-omit-frame-pointer -g")
    set(TSAN_FLAGS "-fsanitize=thread -fPIE -g -static-libtsan")
    set(USAN_FLAGS "-fsanitize=undefined -fno-sanitize-recover=all")
    set(COMMON_FLAGS "-pedantic -Wall -Wextra -Wcast-align -Wsign-compare -Wunused-parameter \
            -fno-strict-aliasing -ffunction-sections -fdata-sections -Wl,-dead_strip")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} ")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} ${ASAN_FLAGS} ${TSAN_FLAGS} ${USAN_FLAGS} -Wdisabled-optimization -Wshadow")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} ${ASAN_FLAGS} ${USAN_FLAGS} -Wdisabled-optimization -Wshadow")
elseif(UNIX)
    Message("Unix system")
    set(ASAN_FLAGS "-fsanitize=address -fno-omit-frame-pointer -g -static-libasan")
    set(TSAN_FLAGS "-fsanitize=thread -fPIE -g -static-libtsan")
    set(USAN_FLAGS "-fsanitize=undefined -fno-sanitize-recover=all -static-libubsan")
    set(COMMON_FLAGS "-pedantic -Wall -Wextra -Wcast-align -Wsign-compare -Wunused-parameter \
            -fno-strict-aliasing -ffunction-sections -fdata-sections -Wl,--gc-sections")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} ")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} ${ASAN_FLAGS} ${TSAN_FLAGS} ${USAN_FLAGS} -Wdisabled-optimization -Wshadow")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -Wdisabled-optimization -Wshadow")
endif()

#include(cmake/flatbuffers.cmake)
add_subdirectory(s000_cmodel/source/firmware)
add_subdirectory(third_party)

set(s900_include_dir
        ${CMAKE_CURRENT_SOURCE_DIR}/s000_cmodel/source/graph
        ${CMAKE_CURRENT_SOURCE_DIR}/s000_cmodel/source/module/memory
        ${CMAKE_CURRENT_SOURCE_DIR}/s000_cmodel/source/module/nputop
        ${CMAKE_CURRENT_SOURCE_DIR}/s000_cmodel/source/module/nputop/kernels
        ${CMAKE_CURRENT_SOURCE_DIR}/s000_cmodel/source/module/nputop/command
        ${CMAKE_CURRENT_SOURCE_DIR}/s000_cmodel/source/module/nputop/register
        ${CMAKE_CURRENT_SOURCE_DIR}/s000_cmodel/source/module/nputop/weight
        ${CMAKE_CURRENT_SOURCE_DIR}/s000_cmodel/source/module/soctop
        ${CMAKE_CURRENT_SOURCE_DIR}/s000_cmodel/source/utils/src
        ${CMAKE_CURRENT_SOURCE_DIR}/Utility
        ${CMAKE_CURRENT_SOURCE_DIR}/Utility/IO
        ${CMAKE_CURRENT_SOURCE_DIR}/Utility/Parse
        )

file(GLOB all_source
        s000_cmodel/source/graph/*.cpp
        s000_cmodel/source/module/memory/*.cpp
        s000_cmodel/source/module/nputop/*.cpp
        s000_cmodel/source/module/nputop/kernels/*.cpp
        s000_cmodel/source/module/nputop/command/*.cpp
        s000_cmodel/source/module/nputop/register/*.cpp
        s000_cmodel/source/module/nputop/weight/*.cpp
        s000_cmodel/source/module/soctop/*.cpp
        s000_cmodel/source/utils/src/*.cpp
        Utility/IO/*.cpp
        Utility/Parse/*.cpp
        Utility/*.cpp
        )

message(STATUS "s900 include directory summary")
foreach(dir ${s900_include_dir})
    message(STATUS "         ${dir}")
endforeach()

message(STATUS "external include directory summary")
foreach(edir ${external_include_dir})
    message(STATUS "         ${edir}")
endforeach()

message(STATUS "source directory summary")
foreach(src ${all_source})
    message(STATUS "         ${src}")
endforeach()

add_executable(s000_model s000_cmodel/source/main.cpp ${all_source})
target_include_directories(s000_model SYSTEM PUBLIC ${external_include_dir})
target_include_directories(s000_model PUBLIC ${s900_include_dir})

