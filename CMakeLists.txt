cmake_minimum_required(VERSION 3.16)
project(reSID LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB PROJECT_SOURCES_ROOT
    "${CMAKE_SOURCE_DIR}/*.c"
    "${CMAKE_SOURCE_DIR}/*.cc"
    "${CMAKE_SOURCE_DIR}/*.cpp"
)
file(GLOB PROJECT_SOURCES_SRC
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.cc"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)
set(PROJECT_SOURCES ${PROJECT_SOURCES_ROOT} ${PROJECT_SOURCES_SRC})

add_executable(test_app ${PROJECT_SOURCES})

target_include_directories(test_app PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_compile_options(test_app PRIVATE
    $<$<CXX_COMPILER_ID:Clang>:-Wall;-Wextra;-Wshadow>
    $<$<CXX_COMPILER_ID:GNU>:-Wall;-Wextra;-Wshadow>
)

if(APPLE)
    target_link_options(test_app PRIVATE
        "-Wl,-map,${CMAKE_BINARY_DIR}/test_app.map"
    )
    find_library(AudioToolbox_FRAMEWORK AudioToolbox)
    find_library(CoreAudio_FRAMEWORK CoreAudio)
    if(AudioToolbox_FRAMEWORK AND CoreAudio_FRAMEWORK)
        target_link_libraries(test_app PRIVATE ${AudioToolbox_FRAMEWORK} ${CoreAudio_FRAMEWORK})
    else()
        message(WARNING "Could not find AudioToolbox/CoreAudio frameworks")
    endif()
endif()
