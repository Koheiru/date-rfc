cmake_minimum_required(VERSION 3.1.3)

project(date-rfc CXX)

set(BUILD_EXAMPLES TRUE CACHE BOOL "Whether to build examples or not")
set(CMAKE_CXX_STANDARD 11)

set(HEADER_FOLDER   "include")
set(EXAMPLE_FOLDER  "examples")

file(GLOB MAIN_HEADERS "${HEADER_FOLDER}/date-rfc/*.h")
file(GLOB IMPL_HEADERS "${HEADER_FOLDER}/date-rfc/details/*.h")
if (MSVC)
    source_group("Header Files" FILES ${MAIN_HEADERS}) 
    source_group("Header Files\\details" FILES ${IMPL_HEADERS}) 
endif()

set(HEADER_FILES "")
list(APPEND HEADER_FILES ${MAIN_HEADERS})
list(APPEND HEADER_FILES ${IMPL_HEADERS})

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
