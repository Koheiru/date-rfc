cmake_minimum_required(VERSION 3.1.3)

project(date-rfc CXX)

set(BUILD_EXAMPLES TRUE CACHE BOOL "Whether to build examples or not")
set(CMAKE_CXX_STANDARD 11)

set(HEADER_FOLDER   "include")
set(EXAMPLE_FOLDER  "examples")
set(HEADER_FILES    "${HEADER_FOLDER}/date/date-rfc.h")

add_library(date-rfc INTERFACE)
target_include_directories(date-rfc INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${HEADER_FOLDER}>
    $<INSTALL_INTERFACE:include >
)

install(DIRECTORY "${HEADER_FOLDER}/" DESTINATION "include/")

if (${BUILD_EXAMPLES})
    add_executable(example 
        ${HEADER_FILES}
        ${EXAMPLE_FOLDER}/example.cpp)
    target_link_libraries(example date-rfc)
endif ()
