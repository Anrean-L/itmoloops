cmake_minimum_required(VERSION 3.24)

project(
    itmoloops
    VERSION 1.0
    DESCRIPTION "C++ Year 2025 labwork5"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ITMOLOOPS_NOTES_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/notes.txt
)

configure_file(
    ${ITMOLOOPS_NOTES_FILE}
    ${CMAKE_CURRENT_BINARY_DIR}/notes.txt
    COPYONLY
)

add_subdirectory(lib)
add_subdirectory(bin)
