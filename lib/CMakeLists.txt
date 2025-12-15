set(CMAKE_CXX_STANDARD 23)

add_library(itmoloops_lib STATIC 
    song_parser/instrument_builder.cpp
    song_parser/effect_builder.cpp
    song_parser/song_parser.cpp
    song_parser/utils.cpp
    song_renderer.cpp
    wav_file.cpp
)
target_include_directories(itmoloops_lib PUBLIC     
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/song_parser
)
