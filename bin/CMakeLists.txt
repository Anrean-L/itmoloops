add_executable(itmoloops main.cpp)

target_link_libraries(itmoloops PRIVATE itmoloops_lib)


target_compile_definitions(itmoloops PRIVATE
    ITMOLOOPS_NOTES_PATH="${CMAKE_BINARY_DIR}/notes.txt"
)